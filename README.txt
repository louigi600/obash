WHY obfuscate shell scripts:
It's generally not a good idea to obfuscate shell scripts, as it burdens maintenance and seriously impedes debugging, but none the less, there may be circumstances where you might want to obfuscate what's being done, and/or some sensitive data in the script itself. Mind you that obfuscated self executing scrips are not as safe as you might want: they have inside them the code to extract the obfuscated script, so any determined person, with time to spare, can extract the script from them. How long that takes depends on the determination, time to spare and how well the obfuscator was made.


WHY this project:
You might be asking: "Why on earth did you not just use shc ?"
Well in my opinion shc has some short comings which are critical for me:
1) any lenthy script can be seen in clear text (with some formatting issues) with ps
2) apparently shc has hardcoded key in the binary which makes it relatively easy to extract the script (this may have been fixed)
3) what I do with interpreter.h shc has hardcoded in the sources making it harder to maintain the interpreter part

The former of these not only defeats having used shc in the first place but also makes an unholy mess of the output of ps.

If I was any better at programng (and mind you I'm a system administrator not a developer) I'd have probabbly fixed shc but I was unable to figure out the inner workings of shc (altough in principle it should be fairly similar to obash) by reading the code/documentation so I gave up on that. I tried contacting the author of shc but that was a no go too. I was intrested on ccsh but the 50 buck pricetag killed it so I gave birth to obash ... and yes unlike shc obash is only meant for bash scripts but could be made to work on other interpreters with a little editing.


Build:
Simply issue 

# make clean
# make 

to compile obash. You can copy the binary manually to wherever you like.


USE:
obash <source bash script>
will produce a binary executable with ".x" suffixed to the input script name.
Just run <source bash script>.x as you would with the <source bash script> itself.

obash -h 
will show a short help message for it's very few flags and parameters.


How it works internally:
obbash takes the input script and aes-256 encodes it, and also base64 encodes the aes cipertex so that it can be used to declare an unsigned char array.
It then produces an intermediate c file which is basically the interpreter (see interpreter.c) , functions, text array containing the cipher text and the main. The intermediate c file is then compiled into an exacutable.
The intermediate c file is built in the following manner (see mk_sh_c function in functions.c):
includes block from interpreter.h
crypted_script variable containing the base64 aes-256 encoded script
serial and uuid variables (empty if non reusable) 
functions block from interpreter.h
main_body block from interpreter.h

See recreate_interpreter_heade script for details on how interpreter.h is created from interpreter.c.


Key and Inizialization Vector for AES-256 encoding:
The key and iv are not hardcoded into the binary (unless you decide to build a reusable static binary with -r flag) but are retreaved each time from the hardware (hence binding it to a machine). In case of a reusable static binary (built wit -r flag) then the uuid and serial are in the binary itself but will be manipulated anyway by makekey and makeiv so that they are not usable immediately should anyone ever inspect the binary itself.
Although the whereabouts from where the serian and uuid are retreaved is tracable and I make no secret of it (machine uuid and srial number for non reusable and random hex digits for reusable) these should be then manipulated in a way that they are not directly usable as is. In the code there is a comment suggestng where this should be done (see makekey and makeiv functions in functions.c): each and every one of you using obash is encouraged to do so. As an example I strip "-" from the uuid and pad short serial numbers to reach the suggested lenght for the cipher used. Look in sections "Suggestions for key and iv Scrambling"


The actual AES encoding:
The part that does the actual AES encoding came right out of the openssl symmetric encription xample from their wiki with very minor changes. It was well docuented and easy to reuse and that's what I did.
See openssl wiki: https://wiki.openssl.org/index.php/EVP_Symmetric_Encryption_and_Decryption
The main part if the example went into mk_sh_c function wile the encoding and decoding functions were left pretty much untouched. The only thing I recall changing was substituting OPENSSL_config with OPENSSL_no_config to avoid not finding openssl.cnf in the same place as where the reusable static binary was built, it's not neaded anyway for AES Symmetric Encryption and Decryption.

Is this foolproof:
Just like shc it is still possible to extract the script it's just a that at least it's not as easy as "ps -ef" (no joke see further down for the output of ps -ef on a shc encoded script) for any lenthy script.

I've made several attempts to omit using named pipe (which introduces an intrinsic weakness) and each time I hit the same limitation: only non iteractive scripts work right because stdin gets broken wnen the child remaps stdin to read from pipe with dup2.
If you have any suggestions on how to avoid this please contact me <louigi600 (at) yahoo (dot) it>.

Having to use maned pipes introduces a weakness which can be exploited by writing some imple code. What you need to do is watche for any newly created pipe in /tmp and dump them before the obash reader child empties them. This will not work work every time but you can loop untill you succeed eventually (miracle of multitasking).

There is another approach that could be used to extract the cleartext script: look at the binary and try to figure out how the key and iv are  scrambled from  uuid and serial number, which in the distributed version is rather simple (and I encourage people to make their personal changes). Once you have key and iv  that are used you could then base64 decode and aes-256-cdc decode the crypted_script vasiable content. If you have the code with wich the obash binary was buildt on, you can obviously extract the script from the bins (but you should also have the sources in that case).

I regard this as being better then seing the clear text script with "ps -ef" with basically just some formatting issues and some blank prefix.
See the output of a simple "ps -ef | grep <name of the shc encoded script> : (I copied in also the blank pefix so scroll down a little)
[root ~]# ps -ef | grep containers
root      2098 16966  0 16:02 pts/0    00:00:00 /usr/local/bin/containers -c                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                 #!/bin/bash?VERSION=14?# rc_containers  Manage the containers?# chkconfig: 345 86 01?# description: Manage the defined containers?#??NAME=$(basename $0)??UBINDS='dev/pts dev/shm dev sys proc var/log/httpd/*'?MBINDS='dev dev/pts dev/shm sys proc var/log/httpd/*'??usage ()?{ echo -e "usage : $NAME <comand> ?supported commands:\t[start|stop [comma separated container list]] ?\t\t\t[mount|umount <container>] ?\t\t\t[services <start|stop> <container>] ?\t\t\t[status]?\t\t\t[version]"?  echo?  if [ "$*" != "" ] ?  then?    echo "Error: $*"?    exit 1?  else?    exit 0?  fi?}??list_mounted_containers ()?{ mount | grep ".-lv_" | grep  -w "/www" |awk '{printf("%s,",$3)}' |sed -e "s/,$//"?}??list_defined_containers ()?{ find /www -maxdepth 1 -type d |grep -vw "/www$" |awk '{printf("%s,",$1)}' |sed -e "s/,$//" ?}??#show_status ()?#{ echo "Mounted containers: "?#  list_mounted_containers | sed -e "s/,/\n/g"?#?#  echo -e "\n"?#  echo "Container active services: "?#  netstat -ntpl |sort -nk 4 |grep -vE "$(ifconfig eth0 |grep -Eo 'inet (addr:)?([0-9]*\.){3}[0-9]*' | grep -Eo '([0-9]*\.){3}[0-9]*')|/rpcbin|/rpc\.sta|^Proto Recv-Q|^Active Internet" | awk '{print $4}'?#?#}??show_status ()?{ echo?#  eval $(grep IPADDR= /etc/sysconfig/network-scripts/ifcfg-eth0)??  for CONTAINER in $(list_mounted_containers | sed -e "s/,/\n/g")?  do?    CIP=$(grep -iw "^ListenAddress" ${CONTAINER}/etc/ssh/sshd_config |sort -u |awk '{print $NF}')?    CPIP=$(curl -s --interface $CIP http://network-tools.com/ |grep 'name="host"'  |sed -e 's/.* value ="\([0-9,.]*\)".*$/\1/')?    echo "Container: $CONTAINER private IP: $CIP public IP: $CPIP"??    echo "Services running in $CONTAINER "?    CNSL=$(netstat -ntpl |grep -w "$CIP"  |grep -vw "$CIP:443" |awk '{printf("%s %s\n",$4,$NF)}' |sort -nk 1 |tr " /" ":")?    for CSERVICE in $CNSL?    do?      eval $(echo "SA=(${CSERVICE})" |tr ":" " ")?      SP=$(ps -eo pid,ppid,cmd |grep -v grep |grep -w "${SA[2]}" |awk '{printf("%s ", $1)}')?      SM=0?      for PID in $SP?      do?        SM=$(echo $SM $(awk '/Pss/ {print "+", $2}' /proc/$PID/smaps) | bc -l)?      done?      echo "${SA[0]}:${SA[1]} ${SA[3]}(${SA[2]}) - $(echo "scale=2; $SM / 1024" |bc -l) Mb  "?    done?    echo?  done?#netstat  -ntlpA inet |awk '{printf("%s %s\n",$4,$NF)}' |grep -vwE "0:111|0:46358|$IPADDR|Local name|only servers" |sort -nk 1 |tr " /" ":"?}??start_container_services ()?{ echo -n "  Starting container services ... "?  chroot $1 << EOF >/dev/null 2>&1?service container start?EOF?  echo done?}??stop_container_services ()?{ echo -n "  Stopping container services ... "?  chroot $1 << EOF >/dev/null 2>&1?service container stop?EOF?  echo done?}??umount_container ()?{ echo -n "  Unmounting container ... "?  STATUS=0?  ( cd $1?    SSTATUS=0?    for DIR in $UBINDS?    do?      if [ $(mount | grep -cw "${1}/$DIR") -ge 1 ] ?      then?        umount $DIR > /dev/null 2>&1 || ((SSTATUS++))?      fi?    done ?    [ $SSTATUS -ne 0 ] && exit 1 || exit 0?  ) || ((STATUS++)) ?  umount $1 > /dev/null 2>&1 || ((STATUS++))?  [ $STATUS -eq 0 ] && echo done || echo fail ?}??mount_container ()?{ echo -n "  Mounting container ... "?  STATUS=0?  mount $1 > /dev/null 2>&1 || ((STATUS++))?  ( cd $1?    SSTATUS=0?    for DIR in $MBINDS?    do?      if [ $(mount | grep -cw "${1}/$DIR") -eq 0 ]?      then?        [ -f $DIR -a ! -f /$DIR ] && touch /$DIR >/dev/null 2>&1?        mount -B /$DIR $DIR || ((SSTATUS++))?      fi?    done?    [ $SSTATUS -ne 0 ] && exit 1 || exit 0?  ) || ((STATUS++))?  [ $STATUS -eq 0 ] && echo done || echo fail ?}??do_stop ()?{ for CONTAINER in $(echo $* | tr "," " " | sed -e 's?/ ? ?g')?  do?    if list_mounted_containers | grep -qw $CONTAINER?    then?      echo "Stoppong $CONTAINER ... "?    else?      echo "$CONTAINER container is not running"?      continue?    fi?    stop_container_services $CONTAINER?    umount_container $CONTAINER?  done?}??do_start ()?{ for CONTAINER in $(echo $* | tr "," " " | sed -e 's?/ ? ?g')?  do?    if list_mounted_containers | grep -qw $CONTAINER?    then?      echo "$CONTAINER container already mounted"?      continue?    else?      echo "Starting $CONTAINER ... "?    fi?    mount_container $CONTAINER?    start_container_services $CONTAINER?  done?}??case $1 in ?  status) show_status ;;?  start) ?    if  [ "$2" != "" ]?    then?      shift ?      do_start $*?    else?      do_start $(list_defined_containers)?    fi?  ;;?  stop) ?    if  [ "$2" != "" ]?    then?      shift ?      do_stop $*?    else?      do_stop $(list_mounted_containers)?    fi?  ;;?  mount) mount_container $2 ;;?  umount) umount_container $2 ;;?  services) [ $# -ne 3 ] && usage?    case $2 in?      start) start_container_services $3 ;;?      stop) stop_container_services $3 ;;?      *)  usage ;;?    esac?  ;;?  version) echo "$NAME version: $VERSION" ; exit 0 ;;?  *) usage ;;?esac? /usr/local/bin/containers status
root      2118  2098  0 16:02 pts/0    00:00:00 /usr/local/bin/containers -c



Suggestions for key and iv Scrambling:
Just in case you are short of ideas on how to manipulate the key and iv to make it hard to decrypt your obfuscated scripts.
One thing you want to avoid is the use of external programs to do the manipulation as that would be easy to trace, you will need to write your own code and include it in the sources. 
Here are some suggestions:
hash the key and iv with md5/sha sum, 
put the key and iv in a matrix and then operate some mathematical transformation on the matrix (like transpose, rotate, traslate, calculate it's invert ...),
base64 encode,
alter the case or add a constant to each char ....




Making a non restricted binary:
I'm working on buiding a binary that can be used on machines different from the one it was obfuscated on, provided the machines are bynary compatible in the first place and that the kernels and glibc are compatible too. I don't want to hit the same limitations that shc has (where you need to be running fairly similar setup) so I'll go the static binary way.
This is still work in progress but I've been able to produfe reusable binaries ... just have not had mich time to test it to any extent.

This is a note for the autor
This produced a static binary with a warning:
gcc -static testme.c -lssl -lcrypto -ldl -lltdl -static-libgcc

/usr/lib64/gcc/x86_64-slackware-linux/5.3.0/../../../../lib64/libcrypto.a(dso_dlfcn.o): In function `dlfcn_globallookup':
dso_dlfcn.c:(.text+0x11): warning: Using 'dlopen' in statically linked applications requires at runtime the shared libraries from the glibc version used for linking

Just a note onn how I strip my personal key and iv scrambling stuff from the version I use myself
diff -ubw functions.c  distributable/functions.c  > from_my_to_distributable.patch
make distributable creates the directory, copies the code  and patches functions.c 


