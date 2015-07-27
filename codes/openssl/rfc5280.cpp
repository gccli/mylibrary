#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdint.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <dirent.h>
#include <getopt.h>

#include <openssl/bio.h>
#include <openssl/err.h>
#include <openssl/x509.h>
#include <openssl/x509v3.h>
#include <openssl/pem.h>

#include <list>
#include <map>
#include <string>
using namespace std;

#pragma pack(1)
struct ACIsection {
    unsigned short flags;
    unsigned int   length;
    unsigned char  hash[20];
};
#pragma pack(0)

class X509Crl;
const char *output_dump = "/tmp/crl.dump";
const char *output_bin = "/tmp/crl.bin";

map<uint64_t, X509Crl *>  CRL_map;

const char *dumpmem(unsigned char *buf, int len)
{
    static char sbuf[1024] = {0};
    int offs = 0;
    for(int i=0; i<len; ++i) {
	offs += sprintf(sbuf+offs, "%.2x", buf[i]);
    }
    return sbuf;
}


class X509Crl 
{
public:
    X509Crl()
	:m_crl(NULL)
	,m_hash(0)
	,m_issuer(NULL)
	,m_rev(NULL)
    {
    }
    ~X509Crl() { 
	m_crls.clear();
	X509_CRL_free(m_crl);
    }

    uint64_t get_hash();
    const char *get_issuer();
    STACK_OF(X509_REVOKED) *get_rev_list();

public:
    bool load(const char *filename);
    bool append(STACK_OF(X509_REVOKED) *rev, const char *filename);
    bool dump(const char *outfile = NULL);
    bool merge();

private:
    X509_CRL *m_crl;
    
    uint64_t m_hash;
    char *m_issuer;

    list<string> m_crls;
    STACK_OF(X509_REVOKED) *m_rev;
};

uint64_t X509Crl::get_hash()
{
    if (m_hash == 0) {
	m_hash = X509_NAME_hash(X509_CRL_get_issuer(m_crl));
    }
    return m_hash;
}

const char *X509Crl::get_issuer()
{
    if (m_issuer == NULL) {
	m_issuer = X509_NAME_oneline(X509_CRL_get_issuer(m_crl), NULL, 0);
    }
    return m_issuer;
}

STACK_OF(X509_REVOKED) *X509Crl::get_rev_list()
{
    m_rev = X509_CRL_get_REVOKED(m_crl); 
    return m_rev;
}

bool X509Crl::load(const char *filename)
{
    bool ret = false;
    if (!filename) 
	return false;
    BIO *in = BIO_new(BIO_s_file());
    if (!in) 
	return false;
    if (BIO_read_filename(in, filename) <= 0) {
	fprintf(stderr, "read file '%s' : %s\n", filename, strerror(errno));
	goto end;
    }
    
    if ((m_crl=d2i_X509_CRL_bio(in,NULL)) == NULL) {
	fprintf(stderr, "Not a default CRL format, try PEM format...");
	if ((m_crl=PEM_read_bio_X509_CRL(in,NULL,NULL,NULL)) == NULL) {
	    fprintf(stderr, " [failed]\n");
	} else {
	    fprintf(stderr, " [ok]\n");
	}
    }

    if (m_crl) {
	ret = true;
	m_crls.push_back(string(filename));
	get_hash();
	get_issuer();
	get_rev_list();
    }
end:
    BIO_free(in);
    return ret;
}

bool X509Crl::append(STACK_OF(X509_REVOKED) *rev, const char *filename)
{
    if (!rev || sk_X509_REVOKED_num(rev) <= 0)
	return false;
    X509_REVOKED *r;

    for(int i=0; i<sk_X509_REVOKED_num(rev); ++i) {
	r = sk_X509_REVOKED_value(rev, i);
	sk_X509_REVOKED_push(m_rev, r);
    }

    m_crls.push_back(string(filename));

    return true;
}


bool X509Crl::dump(const char *outfile)
{
    BIO *out=BIO_new(BIO_s_file());
    if (outfile == NULL) {
	BIO_set_fp(out,stdout,BIO_NOCLOSE);
    } else {
	//BIO_write_filename(out,(void*)outfile);
	BIO_append_filename(out,(void*)outfile);
    }
    X509_REVOKED *r;

    int count = sk_X509_REVOKED_num(m_rev);
//    BIO_printf(out,"Issuer:    %s\n", m_issuer);
    BIO_printf(out,"HASH 0x%08x  (count:%d)\n", m_hash, count);

    if (!sk_X509_REVOKED_is_sorted(m_rev)) {
	sk_X509_REVOKED_sort(m_rev);
    }
    for(int i = 0; i < sk_X509_REVOKED_num(m_rev); i++) {
	r = sk_X509_REVOKED_value(m_rev, i);
	i2a_ASN1_INTEGER(out,r->serialNumber);
	BIO_printf(out, "\n");
    }

    BIO_free_all(out);

    return true;
}

bool X509Crl::merge()
{
    printf("before merge, %d entries, %d files\n", sk_X509_REVOKED_num(m_rev), m_crls.size());

    list<string>::iterator it = m_crls.begin();
    for(; it != m_crls.end(); ++it) {
	printf("  %s\n", it->c_str());
    }
    sk_X509_REVOKED_sort(m_rev);
    int ndup = 0;
    int nent = sk_X509_REVOKED_num(m_rev);
    X509_REVOKED *r, *prev;

    for(int i = 0; i < sk_X509_REVOKED_num(m_rev); i++) {
	r = sk_X509_REVOKED_value(m_rev, i);
	if (!prev) {prev = r;}
	if (ASN1_INTEGER_cmp(prev->serialNumber, r->serialNumber) == 0) {
	    //printf("duplicated\n");
	    ndup++;
	    sk_X509_REVOKED_delete(m_rev, i);
	} else {
	    prev = r;
	}
    }

    printf("after merge, %d duplicated, %d -> %d entries\n", ndup, nent, sk_X509_REVOKED_num(m_rev));
    return true;
}


bool compile()
{
    const char *tmpfile = output_bin;
    FILE *fp = fopen(tmpfile, "wb");
    if (!fp) {
	return false;
    }

    uint32_t bodylen = 0;
    uint32_t totallen = 0;
    struct ACIsection sec;
    memset(&sec, 0, sizeof(sec));
    sec.flags = 2;
    fwrite(&sec, sizeof(sec), 1, fp);

    STACK_OF(X509_REVOKED) *revoked;
    X509_REVOKED *r;
    ASN1_INTEGER *a;
    map<uint64_t, X509Crl *>::iterator it;
    for(it = CRL_map.begin(); it != CRL_map.end(); ++it) {
	revoked = it->second->get_rev_list();
	uint64_t hash = it->first;
	uint32_t count = sk_X509_REVOKED_num(revoked);
	fwrite(&hash, 4, 1, fp);
	fwrite(&count, 4, 1, fp);
	bodylen += 8;
	for(int i=0; i<count; ++i) {
	    r = sk_X509_REVOKED_value(revoked, i);	    
	    a = r->serialNumber;
	    fwrite(&a->length, 1, 1, fp);
	    fwrite(a->data, a->length, 1, fp);
	    bodylen += (1+a->length);
	}
    }

    fclose(fp);

    int fd = open(tmpfile, O_RDWR);

    printf("Body length 0x%x\n", bodylen);
    
    totallen = sizeof(sec) + bodylen;
    void *pmem = mmap(NULL, totallen, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);
    if (pmem == MAP_FAILED) {
	perror("mmap");
	return false;
    }
    sec.length = bodylen;
    SHA1((unsigned char *)pmem+sizeof(sec), bodylen, sec.hash);
    memcpy(pmem, &sec, sizeof(sec));
    printf("  Hash %s\n", dumpmem(sec.hash, 20));
    
    close(fd);
    munmap(pmem, totallen);

    return true;
}

bool decompile(const char *filename)
{
    int ret = false;
    uint32_t offs = 0;
    unsigned char md[20];
    const char *tmpfile = output_bin;

    struct stat st;
    if (stat(tmpfile, &st) < 0) {
	return false;
    }

    FILE *fp = fopen(tmpfile, "rb");
    if (!fp) {
	return false;
    }

    FILE* fout = fopen(filename, "w");
    unsigned char *ss;

    ACIsection sec;
    fread(&sec, 1, sizeof(sec), fp);
    if (sec.length != st.st_size-sizeof(sec)) {
	printf("Invalid file\n");
	goto End;
    } 

    ss = (unsigned char *)malloc(sec.length);
    fread(ss, sec.length, 1, fp);

    SHA1(ss, sec.length, md);
    if (memcmp(md, sec.hash, 20)) {
	printf("corroded file\n");
	printf("  %s vs %s\n", dumpmem(md, 20), dumpmem(sec.hash, 20));

	goto End;
    }

    while(offs < sec.length) {
	uint32_t hash = *(uint32_t *) (ss+offs);
	offs += 4;
	uint32_t count = *(uint32_t *) (ss+offs);
	offs += 4;
	fprintf(fout, "HASH 0x%08x  (count:%d)\n", hash, count);

	unsigned char  len = 0;
	unsigned char *val;
	for(int i=0; i<count; ++i) {
	    len = *(unsigned char *)(ss+offs);
	    offs += 1;
	    val = (unsigned char *)(ss+offs);
	    offs += len;
	    for(int j=0; j<len; ++j) {
		fprintf(fout, "%.2X", val[j]);
	    }
	    fprintf(fout, "\n");
	}
    }
    fclose(fout);
    
    ret = true;
End:
    fclose(fp);
    return ret;
}
static int scanfilter(const struct dirent *ent)
{
    static int num = 0;
    if (!strstr(ent->d_name, ".crl"))
	return 0;
    num++;
    if (num >= 10000) {
	printf("Too many CRL files\n");
	return 0;
    }
    return 1;
}

static int scancrls(const char *dirp)
{
    map<uint64_t, X509Crl *>::iterator it;
    struct dirent **namelist;
    STACK_OF(X509_REVOKED) *revoked;
    char path[1024];
    int n = scandir(dirp, &namelist, scanfilter, alphasort);
    if (n < 0) {
	perror("scandir");
	return -1;
    }
    while (n--) {
	printf("------------ %s ------------\n", namelist[n]->d_name);
	sprintf(path, "%s/%s", dirp, namelist[n]->d_name);
	X509Crl *x = new X509Crl;
	if (!x || !x->load(path)) {
	    delete x;
	    goto NO_REV_LIST;
	}
	revoked = x->get_rev_list();
	if (!revoked || sk_X509_REVOKED_num(revoked) == 0) {
	    delete x;
	    goto NO_REV_LIST;
	}
	it = CRL_map.find(x->get_hash());
	if (it != CRL_map.end()) {
	    it->second->append(revoked, path);
	    //delete x;
	} else {
	    CRL_map.insert(make_pair(x->get_hash(), x));
	}

      NO_REV_LIST:
	free(namelist[n]);
    }
    free(namelist);

    printf("End of scan\n");
    for(it = CRL_map.begin(); it != CRL_map.end(); ++it) {
	it->second->merge();
    }

    for(it = CRL_map.begin(); it != CRL_map.end(); ++it) {
	it->second->dump(output_dump);
    }

    return 0;
}


int main(int argc, char *argv[])
{
    int c;


    while (1) {
	//int this_option_optind = optind ? optind : 1;
	int option_index = 0;
	static struct option long_options[] = {
	    {"dir", 1, 0, 0},
	    {"dec", 1, 0, 0},
	    {0, 0, 0, 0}
	};

	c = getopt_long(argc, argv, "d:",
                        long_options, &option_index);
	if (c == -1)
	    break;

	switch (c) {
	    case 0:
		if (strcmp("dir", long_options[option_index].name) == 0) {
		    truncate(output_dump, 0);
		    scancrls(optarg);
		    compile();
		}
		else if (strcmp("dec", long_options[option_index].name) == 0) {
		    decompile(optarg);
		}
		break;

	    case 'd':
		X509Crl x;
		if (x.load(optarg))
		    x.dump();
		break;
	}
    }

    return 0;
}
