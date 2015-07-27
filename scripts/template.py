#! /usr/bin/env python


def main():
    try:
        opts,args = getopt.getopt(sys.argv[1:], "a:b:m:", ["address=", "bcast=", "mask=", "cidr="])
    except getopt.GetoptError as err:
        print str(err)
        sys.exit(2)

    for o, a in opts:
        if o in ("-a", "--address"):
            addr=a
        elif o in ("-b", "--bcast"):
            bcast=a
        elif o in ("-m", "--mask"):
            mask=a
        elif o in ("--cidr"):
            cidr=a
        else:
            sys.exit(2)

if __name__ == "__main__":
    main()

