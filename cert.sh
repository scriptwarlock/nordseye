#!/bin/bash

echo "Generating SSL/TLS certificates"

PWD=`pwd`
TMPDIR="cert_tmp"
mkdir $TMPDIR
mkdir -p $TMPDIR/demoCA/private/
mkdir -p $TMPDIR/demoCA/newcerts
mkdir -p $TMPDIR/demoCA/certs
mkdir -p $TMPDIR/demoCA/crl
cd $TMPDIR
touch demoCA/index.txt
echo "01" > demoCA/serial

echo "generate self-signed CA ... "
echo -e ".\n.\n.\n.\n.\n`hostname`\n.\n" | \
    openssl req -new -x509 -nodes \
                -keyout demoCA/private/cakey.pem \
                -out demoCA/cacert.pem -days 3650
echo "done"

echo "generate certificate and sign request ... "
echo -e  ".\n.\n.\n.\n.\nccl\n.\n\n\n" | \
    openssl req  -new -nodes \
                 -keyout key.pem -out newreq.pem \
                 -days 3650
cat newreq.pem key.pem > new.pem
echo "done"

echo "sign certificate with newly created CA ... "
echo -e "y\ny\n" | openssl ca \
    -policy policy_anything \
    -out cert.pem -infiles new.pem
sleep 2
echo "done"

cp demoCA/cacert.pem CAcert.pem
cp CAcert.pem ../CA.pem
cat key.pem cert.pem > ../cert.pem
cd $PWD
rm -rf $TMPDIR
echo -e "\n\nCopy cert.pem and CA.pem to ~/.mkahawa on the server and to ~/.mkahawa on the clients\n"

