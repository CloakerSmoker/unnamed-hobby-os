import sys, os

mode = sys.argv[1]
busybox = sys.argv[2]

links = open(f'{busybox}/busybox.links', 'r').readlines()

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