package test;

use strict;
require Exporter;
our @ISA = qw(Exporter);
our @EXPORT_OK = qw(feedback_parse_file);

use constant {
    FEEDBACK_SRCID_RTSS => 0,
    FEEDBACK_SRCID_AD => 1,
    FEEDBACK_SRCID_AE => 2, # reserve for AE
    FEEDBACK_SRCID_RTC => 3,
    FEEDBACK_SRCID_SOPHOS => 4, # reserve for Sophos
    FEEDBACK_SRCID_SPIE => 5,
    FEEDBACK_SRCID_MIDE => 6,
    FEEDBACK_SRCID_ICE => 7,
    FEEDBACK_SRCID_ASH => 8,
    FEEDBACK_SRCID_SCANFLOW => 128,
    FEEDBACK_SRCID_ACI => 129,
    FEEDBACK_SRCID_STAT => 192,
};

use constant {
    FEEDBACK_ITEM_TYPY_VERSION => 1,
    FEEDBACK_ITEM_TYPE_INDICATOR => 2,
    FEEDBACK_ITEM_TYPE_URL => 3,
    FEEDBACK_ITEM_TYPE_SIGID => 4,
    FEEDBACK_ITEM_TYPE_SIGVISION => 5,
    FEEDBACK_ITEM_TYPE_SLOW_URL_CONTENT_SIZE => 6,
    FEEDBACK_ITEM_TYPE_SLOW_URL_TIME => 7,
    FEEDBACK_ITEM_TYPE_SLOW_URL_ANALYTICS_ID => 8,
    FEEDBACK_ITEM_TYPE_SPIE_OBJECT_NUM => 9,
    FEEDBACK_ITEM_TYPE_SPIE_BITMAP_RESULT => 10,
    FEEDBACK_ITEM_TYPE_STAT => 11
};

sub my_time2str {
    my $t = $_[0];
    my ($sec,$min,$hour,$mday,$mon,$year,$wday,$yday,$isdst);
    ($sec,$min,$hour,$mday,$mon,$year,$wday,$yday,$isdst) = localtime($t);
    my $strt = sprintf("%02d/%02d %02d:%02d:%02d", $mon+1,$mday,$hour,$min,$sec);
   
    return $strt;
}


sub my_unpack {
    my $buf = $_[0];
    my $len = $_[1];

    my $fmt = sprintf("@%da*", $len); 
    $buf = unpack($fmt, $buf);

    return $buf;
}

sub my_dump_arrayref {
    my $offs = $_[0];
    my $aref = $_[1];

    my @A = @$aref;
    my $isscalar = 0;
    my $tag = sprintf("%*s", $offs, "# ");

    for (my $i=0; $i <= $#A; $i++)
    {
	if (ref($aref->[$i]) eq "ARRAY")
	{
	    $isscalar=0;
	    my_dump_arrayref($offs+2, $aref->[$i]);
	}
	elsif (ref($aref->[$i]) eq "HASH")
	{
	    $isscalar=0;
	    my_dump_hashref($offs+2, $aref->[$i]);
	}
	else
	{
	    if (!$isscalar) {
		printf("%s", $tag);
	    }	    
	    $isscalar=1;
	    printf("%s ", $aref->[$i]);
	}
    }
    if ($isscalar) {
	print "\n";
    }
}

sub my_dump_hashref {
    my $offs = $_[0];
    my $href = $_[1];

    my %f = %$href;
    my $tag = sprintf("%*s", $offs, "+ ");

    foreach my $key (keys %f)
    {
	my $value = $f{$key};
	if (ref($value) eq "ARRAY") {                                                                                                                                                                                                                                 
	    printf("%s$key :\n", $tag);                                                                                                                                                                                                                               
	    my_dump_arrayref($offs+2, $value);                                                                                                                                                                                                                        
	} elsif (ref($value) eq "HASH") {                                                                                                                                                                                                                             
	    printf("%s$key :\n", $tag);                                                                                                                                                                                                                               
	    my_dump_hashref($offs+2, $value);                                                                                                                                                                                                                         
	} else {
	    if ($key eq 'time') {
		printf("%s%-9s : %s\n", $tag,$key,my_time2str($value));
	    } else {
#		printf("%s%-9s : $value\n", $tag,$key);
		printf("%s%-9s : ", $tag,$key);
		print(" $value");
		print(" \n");
	    }
	}
    }
} 

sub my_dump_chunk {
    my $params = shift;
    my %chunk = %$params;

    my %fsrc = (0 => 'RTSS', 1 => 'AD', 2 => 'AE', 3 => 'RTC', 4 => 'Sophos', 5 => 'SPIE', 
		6 => 'MIDE', 7 => 'ICE', 8 => 'ASH', 128 => 'SCANFLOW', 129 => 'ACI', 192 => 'STAT');

    printf(" | Attribute len : $chunk{attrlen}\n"); 
    printf(" | Feed Number   : $chunk{num}\n"); 
    printf(" | Time Stamp    : %s\n", my_time2str($chunk{ts})); 

    foreach my $key (keys %chunk)
    {
	my $value = $chunk{$key};
	if ($key eq 'feedlist') {
	   my $array_ref = $chunk{feedlist};
	   my @aref = @$array_ref;

	   for(my $i=1; $i <= $#aref; $i++) {
	       my $f_id = $array_ref->[$i]->{feed_source_id};
	       my $f_len= $array_ref->[$i]->{feed_len};
	       printf("*** %s Feedback[$i] Length:$f_len\n", $fsrc{$f_id});
	       my_dump_hashref(6, $array_ref->[$i]);
	   }
	} if ($key eq 'attribute') {
	    printf(" | Attribute     : \n"); 
	    my $hash_ref = $chunk{attribute}; 
	    my %href = %$hash_ref;
	    my_dump_hashref(6, $hash_ref);
	}
    }
}
sub my_dump_buffer {
    my $params = shift;
    my %buffer = %$params;

    printf(" | Feedback Version : $buffer{fbver}\n"); 
    printf(" | CCA lib Version  : $buffer{libver}\n"); 
    my $cnt = $buffer{vpnum};
    if ($cnt > 0) {
	printf(" | Engine Name | Version | DB Version\n"); 
    }
    for (my $j=0; $j<$cnt; $j++) {
	my $hengine = $buffer{engine_array}[$j];
	printf("   %8s    | %4s    | %-8s\n", 
	       $hengine->{engname}, $hengine->{engver}, $hengine->{dbver});
    }
    printf(" | Time Stamp : %s\n", my_time2str($buffer{ts})); 

    my $array_ref = $buffer{chunklist};
    my @aref = @$array_ref;
    for(my $i=1; $i <= $#aref; $i++) {
	printf("** CHUNK[$i]\n");
	my_dump_chunk($array_ref->[$i]);
    }
}

sub my_dump_buffer_list {
    my $array_ref = shift;
    my @aref = @$array_ref;
    for(my $i=1; $i <= $#aref; $i++) {
	printf("* BUFFER[$i]\n");
	my_dump_buffer($array_ref->[$i]);
    }
}

sub feedback_parse_rtss_ad {
    my $buf = $_[0];
    my $len = $_[1];
    my $src = $_[2]; 
    my $content = {};
    if ($src == FEEDBACK_SRCID_RTSS) {
	$content->{rtss} = 1;
    } else {
	$content->{ad} = 1;
    }
    my $data = substr($buf, 0, $len);
    my %format = ( 1 => "c", 2 => "s", 3 => "l", 4 => "l" );
    while(length($data) && (my ($type, $buffer) = unpack("cs/a*", $data))) {
	my $length = length($buffer);
	substr($data, 0, 3 + $length, "");
	
	if ($type == FEEDBACK_ITEM_TYPY_VERSION) {
	    my $v = unpack($format{$length}, $buffer);
	    $content->{version} = $v;
	} elsif ($type == FEEDBACK_ITEM_TYPE_INDICATOR) {
	    my $indicator = unpack($format{$length}, $buffer);
	    $content->{indicator} = $indicator;	
	} elsif ($type == FEEDBACK_ITEM_TYPE_URL) {
	    my $url = unpack("a*", $buffer);
	    $content->{url} = $url;
	} elsif ($type == FEEDBACK_ITEM_TYPE_SIGID) {
	    my $id = unpack($format{$length}, $buffer);
	    push(@{$content->{id}}, $id);
	} elsif ($type == FEEDBACK_ITEM_TYPE_SIGVISION) {
	    my $revision = unpack($format{$length}, $buffer);
	    push(@{$content->{revision}}, $revision);
	} elsif ($type == FEEDBACK_ITEM_TYPE_SLOW_URL_CONTENT_SIZE) {
	    my $content_size = unpack($format{$length}, $buffer);
	    $content->{content_size} = $content_size;
	} elsif ($type == FEEDBACK_ITEM_TYPE_SLOW_URL_TIME) {
	    my $scan_time = unpack($format{$length}, $buffer);
	    $content->{scan_time} = $scan_time;
	} elsif ($type == FEEDBACK_ITEM_TYPE_SLOW_URL_ANALYTICS_ID) {
            my $scan_type = unpack("a*", $buffer);
            $content->{scan_type} = $scan_type;
        } else {
	    printf("Warning - Unknown Type $type\n");
	}
    }

    return $content;
}

sub feedback_parse_rtc {
    my $buf = $_[0];
    my $len = $_[1];

    if ($len == 5120 || $len == 10240) {
	printf("Error: Record truncated!");
	return undef;
    }

    my $content = {rtc => 1};
    my $data = substr($buf, 0, $len);

    my ($dbver, $time, $globalrate, $number_rtc_hits);
    ($dbver, $time, $globalrate, $number_rtc_hits, $data) = unpack("LLfLa*", $data);

    $content->{dbver} = $dbver;
    $content->{time} = $time;
    $content->{globalrate} = $globalrate;
    # TODO call ACR_DB_DB3 module & using classifier_models

    # pull out the classifier list
    for (my $i=0;$i<$number_rtc_hits;$i++) {
        my ($classifier_id, $score, $rate);
        ($classifier_id, $score, $rate, $data) = unpack("Sffa*", $data);

        # add to map of classifiers	
        my $crow;
        $crow->{cid} = $classifier_id;
	# TODO using classifier_models
        # $crow->{model} $classifier_models->{$dbver}->{$classifier_id} || 1;
        $crow->{score} = $score;
        $crow->{rate} = $rate;
        $content->{classifiers}->{$classifier_id} = $crow;
    }

    # pull out vector size    
    my ($vid, $totalscore, $vector_size);
    ($vid, $totalscore, $vector_size, $data)=unpack("LfLa*",$data);
    $content->{vid} = $vid;
    $content->{totalscore} = $totalscore;

    # pull out the vector of phash ids    
    my @vinfo;
    for (my $i=0;$i<$vector_size;$i++) {
        my ($phash_id, $phash_count);
        ($phash_id, $phash_count, $data) = unpack("Lfa*",$data);

        # add phash/freq
        my $pair;
        $pair->{phash_id} = $phash_id;
        $pair->{phash_count} = $phash_count;
        push @{$content->{classifiers}->{$vid}->{vector}}, $pair;
    }

    # a hash was added
    my $remaining_data = length($data);
    if ($remaining_data >= 20)
    {
        my $bdata;
        ($bdata, $data)=unpack("a20a*", $data);
        my @bytes=unpack("C20", $bdata);

        $content->{hash} = '';
        for (my $i=0; $i < 20; ++$i)
        {
            $content->{hash} = $content->{hash} . sprintf('%02x', $bytes[$i]);
        }

        # a timeout was added
        $remaining_data = length($data);
        if ($remaining_data >= 4)
        {
            my $elapsed;
            ($elapsed, $data)=unpack("fa*", $data);

            $content->{timeout} = ($elapsed > 0) ? 1 : 0;
            $content->{elapsed} = $elapsed;
        }
    }

    return $content;
}

sub feedback_parse_spie {
    my $buf = $_[0];
    my $len = $_[1];
    my $content = {spie => 1};

    my $data = substr($buf, 0, $len);
    my %format = ( 1 => "c", 2 => "s", 3 => "l", 4 => "l" );
    while(length($data) && (my ($type, $buffer) = unpack("cs/a*", $data))) {
	my $length = length($buffer);
	substr($data, 0, 3 + $length, "");
	
	if ($type == FEEDBACK_ITEM_TYPY_VERSION) {
	    my $v = unpack($format{$length}, $buffer);
	    $content->{version} = $v;
	} elsif ($type == FEEDBACK_ITEM_TYPE_INDICATOR) {
	    my $indicator = unpack($format{$length}, $buffer);
	    $content->{indicator} = $indicator;	
	} elsif ($type == FEEDBACK_ITEM_TYPE_URL) {
	    my $url = unpack("a*", $buffer);
	    $content->{url} = $url;
	} elsif ($type == FEEDBACK_ITEM_TYPE_SLOW_URL_CONTENT_SIZE) {
	    my $content_size = unpack($format{$length}, $buffer);
	    $content->{content_size} = $content_size;
	} elsif ($type == FEEDBACK_ITEM_TYPE_SLOW_URL_TIME) {
	    my $scan_time = unpack($format{$length}, $buffer);
	    $content->{scan_time} = $scan_time;
	} elsif ($type == FEEDBACK_ITEM_TYPE_SPIE_OBJECT_NUM) {
	    my $object_number = unpack($format{$length}, $buffer);
	    $content->{object_number} = $object_number;
	} elsif ($type == FEEDBACK_ITEM_TYPE_SPIE_BITMAP_RESULT) {
	    my $result_map = unpack($format{$length}, $buffer);
	    $content->{result_map} = $result_map;
	} else {
	    printf("Warning - Unknown Type $type\n");
	}
    }
    return $content;
}

sub feedback_parse_mide {
    my $buf = $_[0];
    my $len = $_[1];
    my $content = {mide => 1};

    my $data = substr($buf, 0, $len);

    my $vector_size;
    ($vector_size, $data) = unpack("La*", $data);
    for (my $vector_count = 0; $vector_count < $vector_size; ++$vector_count)
    {
        my $vector_data;
        ($vector_data, $data) = unpack("L/Aa*", $data);

        push @{$content->{vector}}, $vector_data;
    }

    my $map_size;
    ($map_size, $data) = unpack("La*", $data);
    for (my $map_count = 0; $map_count < $map_size; ++$map_count)
    {
        my ($tree_number, $tree_decision);
        ($tree_number, $tree_decision, $data) = unpack("LLa*", $data);

        $content->{map}->{$tree_number} = $tree_decision;
    }

    my ($iframe, $ratio, $dbversion);

    ($iframe, $data) = unpack("L/Aa*", $data);
    $content->{iframe} = $iframe;

    ($ratio, $data) = unpack("fa*", $data);
    $content->{tree_ratio} = $ratio;

    ($dbversion, $data) = unpack("La*", $data);
    $content->{datafile} = $dbversion;

    return $content;
}
sub feedback_parse_ice {
    my $buf = $_[0];
    my $len = $_[1];
    my $content = {ice => 1};

    my $data = substr($buf, 0, $len);
    my ($count, $dbver);
    ($dbver, $count, $data) = unpack("LLa*", $data);
    $content->{dbver} = $dbver;
    $content->{count} = $count;
    my $feeds = {};
    
    for (my $i=0; $i<$count; $i++) {
	my ($hit, $module_id, $catagory, $score, $threshold, $relevance, $action_count);
	($hit, $module_id, $catagory, $score, $threshold, $relevance, $action_count, $data) = unpack("bLSfffLa*", $data);
	$feeds->{$module_id}->{hit} = $hit;
	$feeds->{$module_id}->{module_id} = $module_id;
	$feeds->{$module_id}->{catagory} = $catagory;
	$feeds->{$module_id}->{score} = $score;
	$feeds->{$module_id}->{threshold} = $threshold;
	$feeds->{$module_id}->{relevance} = $relevance;
	$feeds->{$module_id}->{action_count} = $action_count;
	for(my $j=0; $j<$action_count; $j++) {
	    my $action_id;
	    ($action_id, $data) = unpack("La*", $data);
	    push(@{$feeds->{model$module_id}->{action_id}}, $action_id);
	}
    }

    $content->{feeds} = $feeds;
    
    return $content;
}

sub feedback_parse_scanflow {
    my $buf = $_[0];
    my $len = $_[1];
    my $content = {scanflow => 1};

    my $data = substr($buf, 0, $len);
    my ($dbver, $section, $seg_priority, $count);
    ($dbver, $section, $seg_priority, $count, $data) = unpack("LSSLa*", $data);
    $content->{dbver} = $dbver;
    $content->{section} = $section;
    $content->{seg_priority} = $seg_priority;
    $content->{count} = $count;
    my $feeds = {};
    
    for (my $i=0; $i<$count; $i++) {
	my ($segment, $rule_id, $rule_priority);
	($segment, $rule_id, $rule_priority, $data) = unpack("SLSa*", $data);
	$feeds->{$rule_id}->{segment} = $segment;
	$feeds->{$rule_id}->{rule_id} = $rule_id;
	$feeds->{$rule_id}->{rule_priority} = $rule_priority;
    }

    $content->{feeds} = $feeds;
    
    return $content;
}

sub feedback_parse_aci {
    my $buf = $_[0];
    my $len = $_[1];
    my $content = {aci => 1};

    my $data = substr($buf, 0, $len);
    my ($url_len, $url);
    ($url_len, $url) = unpack("La*", $data);
#    $content->{url_len} = $url_len;
    $content->{CRL_url} = $url;

    return $content;
}

sub feedback_parse_stat {
    my $buf = $_[0];
    my $len = $_[1];
    my $content = {statistics => 1};
    my $data = substr($buf, 0, $len);

    my $indicator = 0;
    my $key;
    my $val;
    while(length($data) && (my ($type, $buffer) = unpack("cs/a*", $data))) {
        my $length = length($buffer);
        substr($data, 0, 3 + $length, "");

        if ($type == FEEDBACK_ITEM_TYPE_STAT) {
	    if ($indicator == 0) {
		$key = unpack("a*", $buffer);
		$indicator = 1;
		#printf("key   %s\n", $key);
	    } else {
		$val = unpack("a*", $buffer);
		#printf("value %s\n", $val);
		$indicator = 0;
		$content->{$key} = $val;
	    }
        } else {
            printf("Warning - Unknown Type $type\n");
        }
    }
    return $content;
}

sub feedback_parse_ash {
    my $buf = $_[0];
    my $len = $_[1];
    my $content = {ash => 1};

    my $data = substr($buf, 0, $len);
#| 2 bytes           | 8 bytes      | 40 bytes  | 4 bytes            |
#|-------------------+--------------+-----------+--------------------|
#| signature version | signature id | sha1 hash | signature catagory |

    my ($sigver, $sigid, $sigkey, $sigcat);
    ($sigver, $sigid, $sigkey, $sigcat, $data) = unpack("SQa40L", $data);

    $content->{sigver} = $sigver;
    $content->{sigid}  = $sigid;
    $content->{sigkey} = $sigkey;
    $content->{sigcat} = $sigcat;

    return $content;
}

sub feedback_parse_ae {
    my $buf = $_[0];
    my $len = $_[1];
    my $content = {ae => 1};

    return $content;
}

sub feedback_parse_sophos {
    my $buf = $_[0];
    my $len = $_[1];
    my $content = {sophos => 1};

    return $content;
}



sub feedback_parse_feed_list {
    my $buf = $_[0];
    my $len = $_[1];

    my $count=0;
    my @feedlist = [];

    while($len > 0) {
	my %my_feed;
	my ($srcid, $flen); 
	($srcid, $flen, $buf) = unpack("SLa*", $buf);
#	printf("    BEGIN PARSER $count FEED:$srcid ($flen)\n");

	$my_feed{feed_source_id} = $srcid;
	$my_feed{feed_len} = $flen;
	$my_feed{feed_content} = {};

	if ($srcid == FEEDBACK_SRCID_RTSS || $srcid == FEEDBACK_SRCID_AD) {
	   $my_feed{feed_content} = feedback_parse_rtss_ad($buf, $flen, $srcid); 
	} elsif ($srcid == FEEDBACK_SRCID_AE) {
	   $my_feed{feed_content} = feedback_parse_ae($buf, $flen);
	} elsif ($srcid == FEEDBACK_SRCID_RTC) {
	   $my_feed{feed_content} = feedback_parse_rtc($buf, $flen);
	} elsif ($srcid == FEEDBACK_SRCID_SOPHOS) {
	   $my_feed{feed_content} = feedback_parse_sophos($buf, $flen);
	} elsif ($srcid == FEEDBACK_SRCID_SPIE) {
	   $my_feed{feed_content} = feedback_parse_spie($buf, $flen);
	} elsif ($srcid == FEEDBACK_SRCID_MIDE) {
	   $my_feed{feed_content} = feedback_parse_mide($buf, $flen);
	} elsif ($srcid == FEEDBACK_SRCID_ICE) {
	   $my_feed{feed_content} = feedback_parse_ice($buf, $flen);
	} elsif ($srcid == FEEDBACK_SRCID_ASH) {
	   $my_feed{feed_content} = feedback_parse_ash($buf, $flen);
	} elsif ($srcid == FEEDBACK_SRCID_SCANFLOW) {
	   $my_feed{feed_content} = feedback_parse_scanflow($buf, $flen);
	} elsif ($srcid == FEEDBACK_SRCID_ACI) {
	   $my_feed{feed_content} = feedback_parse_aci($buf, $flen);
	} elsif ($srcid == FEEDBACK_SRCID_STAT) {
	   $my_feed{feed_content} = feedback_parse_stat($buf, $flen);
	}
	$count++;
	push(@{feedlist}, \%my_feed);

	$buf = my_unpack($buf, $flen);
	$len -= 8+$flen;
    }

    return @feedlist;
}

sub feedback_parse_chunk_attribute {
    my $buf = $_[0];
    my $len = $_[1];

    my $data = substr($buf, 0, $len);
    my %format = ( 1 => "c", 2 => "s", 3 => "l", 4 => "l" );
    my %attribute; 

    while(length($data) && (my ($type, $buffer) = unpack("cs/a*", $data))) {
        my $length = length($buffer);
        substr($data, 0, 3 + $length, "");
	if ($type == 1) {
	    my $url = unpack("A*", $buffer);
	    $attribute{url} = $url;	    
	} elsif($type == 2) {
	    my $refurl = unpack("A*", $buffer);
	    $attribute{refurl} = $refurl;	    
	} elsif($type == 3) {
	    my $ua = unpack("A*", $buffer);
	    $attribute{ua} = $ua;
	} elsif($type == 4) {
	    my $custom_id = unpack("A*", $buffer);
	    $attribute{custom_id} = $custom_id;	    
	} elsif($type == 5) {
	    my $user_id = unpack("A*", $buffer);
	    $attribute{user_id} = $user_id;
	} elsif($type == 6) {
	    my $product_name = unpack("A*", $buffer);
	    $attribute{product_name} = $product_name;
	} elsif($type == 7) {
	    my $product_ver = unpack("A*", $buffer);
	    $attribute{product_ver} = $product_ver;
	} elsif($type == 8) {
	    my $filename = unpack("A*", $buffer);
	    $attribute{filename} = $filename;
	} elsif($type == 9) {
	    my $url_hash = unpack("A*", $buffer);
	    $attribute{url_hash} = $url_hash;
	} elsif($type == 10) {
	    my $result_attribute = unpack("L", $buffer);
	    $attribute{result_attribute} = $result_attribute;
	} elsif($type == 11) {
	    my %result;
	    ($result{categoryid},$result{signatureid},$result{threatname}) = unpack("LLA*", $buffer);
	    push(@{$attribute{CCA_Scan_Result}}, \%result);
	} elsif($type == 12) {
	    my $direction = unpack("L", $buffer);
	    $attribute{direction} = $direction;
	} else {
	    printf("Warning - Invalid type $type\n");
	}
    }
    
    return %attribute;
}

sub feedback_parse_chunk_list {
    my $buf = $_[0];
    my $len = $_[1];
    my $count=0;
    my $hlen; # header length
    my $clen; # content length

    my @chunklist = [];

    while($len > 0) {
	$hlen = 0;

	my ($ts, $type, $num, $attrlen);
	($ts, $type, $num, $attrlen, $buf) = unpack("LSSLa*",$buf);

	my %my_chunk;

	$my_chunk{ts} = $ts;
	$my_chunk{type} = $type;
	$my_chunk{num} = $num;
	$my_chunk{attrlen} = $attrlen;

	$hlen += 16;
	if ($attrlen > 0) {
	    my %my_attri;
	    %my_attri = feedback_parse_chunk_attribute($buf, $attrlen);
	    $my_chunk{attribute} = \%my_attri;
	    $buf = my_unpack($buf, $attrlen); # skip attribute 
	}
	$hlen += $attrlen;
	($clen, $buf) = unpack("La*", $buf);

	printf("  BEGIN PARSER CHUNK[$count], $num feeds exists, length $clen\n");
	my @feedlist = [];
	
	@feedlist = feedback_parse_feed_list($buf, $clen);
	$my_chunk{feedlist} = \@feedlist;
	printf("  END PARSER CHUNK[$count], $#feedlist feeds, dump chunk\n");

	$count++;
	$buf = my_unpack($buf, $clen); # skip content

	$len -= $hlen+$clen;
	push(@{chunklist}, \%my_chunk);
    }

    return @chunklist;
}

sub feedback_parse_buffer {
    my $in_file_buf = $_[0];
    my $in_file_len = $_[1];

    my @feedback_buffer_list = [];

    while($in_file_len > 0) {
	my %my_buffer;
	my $fbver;
	my $libver;
	my $verpairnum;

	($fbver, $in_file_buf) = unpack("A6a*",$in_file_buf);
	($libver, $in_file_buf) = unpack("A8a*",$in_file_buf);
	($verpairnum, $in_file_buf) = unpack("Sa*",$in_file_buf);

	$my_buffer{fbver} = $fbver;
	$my_buffer{libver} = $libver;
	$my_buffer{vpnum} = $verpairnum;
	$my_buffer{engine_array} = [];

	for (my $i=0; $i<$verpairnum; $i++) {
	    my ($engname, $engver, $dbver);
	    ($engname, $engver, $dbver, $in_file_buf) = unpack("A4A4A8a*", $in_file_buf);
	    $my_buffer{engine_array}[$i]->{engname} = $engname;
	    $my_buffer{engine_array}[$i]->{engver} = $engver;
	    $my_buffer{engine_array}[$i]->{dbver} = $dbver;
	}

	my ($ts, $in_file_buflen);
	($ts, $in_file_buflen, $in_file_buf) = unpack("LLa*", $in_file_buf);

	$my_buffer{ts} = $ts;
	$my_buffer{len} = $in_file_buflen;
	my @chunklist = [];

	printf(" BEGIN PARSER BUFFER: LENGTH $in_file_len \n");
	@chunklist = feedback_parse_chunk_list($in_file_buf, $in_file_buflen);
	printf(" END PARSER BUFFER: $#chunklist chunks\n");

	$my_buffer{chunklist} = \@chunklist;

	$in_file_buf = my_unpack($in_file_buf, $in_file_buflen);
	$in_file_len = length($in_file_buf);

	push (@feedback_buffer_list, \%my_buffer);
    }

    return @feedback_buffer_list;
};

sub feedback_parse_file {
    my $filename = $_[0];
    open(IN, "<", $filename) or die "cannot open < $filename: $!";
    binmode(IN);

    my $len = -s $filename;
    my $ret = read(IN, my $buffer, $len);

    my @buffer_array;

    if ($ret == $len) {
#	printf("BEGIN PARSER FILE: LENGTH $len \n");
	@buffer_array = feedback_parse_buffer($buffer, $len);
#	printf("END PARSE FILE, $#buffer_array buffers\n");

	my_dump_buffer_list(\@buffer_array);

    } else {
	printf("failed to read file\n");
    }
#   printf("\n\ndone\n");
}

1;

