import sys, os

mode = sys.argv[1]
busybox = sys.argv[2]

try:
    links = open(f'{busybox}/busybox.links', 'r').readlines()
except:
    sys.exit(0)

for link in [l.rstrip() for l in links]:
    binary = f'busybox_{os.path.basename(link).upper()}'
    src = f'{busybox}/{binary}'
    dst = link

    if mode == '--src':
        print(src)
    elif mode == '--dst':
        print(dst)
    elif mode == '--install':
        print(f'install /host/{src} /root{dst}')