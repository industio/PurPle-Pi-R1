#!/usr/bin/python
#SigmaStar trade secret
import os, sys, getopt, struct
from Crypto.Cipher import AES
from Crypto.Hash import SHA256
from Crypto.Signature import PKCS1_v1_5
from Crypto.PublicKey import RSA
from Crypto.Util.number import long_to_bytes

image_headers = {'IPL': 0x5F4C5049, 'IPL_CUST': 0x434C5049, 'IPL_CUSTK': 0x4e4C5049, 'uImage': 0x27051956, 'RTK': 0x5F4B5452}

def usage():
    print 'Usage: key_proc.py [--insert|--sign] [--rsa=public_key] [-f bin_file]'
    #print 'While encryption, [--encrypt] [--aes=cipher] [-f bin_file].'
    print 'generate IPLX flow:'
    print '1. key insertion, [--insert] [--rsa=public_key] [-f IPL_CUST.bin]'
    print '2. generate signature, [--sign] [--rsa=private_key] [-f IPL_CUST.key.bin]'
    print '3. image merge, [--merge] [--ipl=IPL.bin] [--iplc=IPL_CUST.key.sig.bin]'
    sys.exit(0)

def indentify_header(message):
    hash = message

    if 0 < len(message):
        for name, header in image_headers.items():
            if header == struct.unpack('I', message[4:8])[0]:
                print '%s' % name
                if 'IPL_CUSTK' == name:
                    lenN, lenAES = struct.unpack('2H', message[16:20])
                    if 0 == lenN and 0 == lenAES:
                        print 'Need to insert key first'
                        return
            elif header == struct.unpack('>I', message[0:4])[0]:
                print '%s' % name
                hash = message[64:]
            elif header == struct.unpack('I', message[32:36])[0]:
                print '%s' % name
                
    return hash
def do_merge(ipl, ipl_cust):
    ipl_bin = None
    ipl_cust_bin = None
    try:
        print "IPL_CUST offset: 0x%04x" % (os.stat(ipl).st_size)
        print "IPL_CUST size: 0x%04x" % (os.stat(ipl_cust).st_size)
        with open(os.path.abspath(ipl), 'rb') as bin:
            ipl_bin = bin.read()
        with open(os.path.abspath(ipl_cust), 'rb') as bin:
            ipl_cust_bin = bin.read()
        with open("IPLX.bin", 'wb') as bin:
            bin.write(ipl_bin + ipl_cust_bin)
        print "IPLX.bin size: 0x%04x" % os.stat("IPLX.bin").st_size
    except BaseException as e:
        print str(e)
        pass
def do_sign(key, bin):
    message = None
    privateKey = None
    hash = None
    try:
        with open(os.path.abspath(bin), 'rb') as msg:
            message = msg.read()
            hash = indentify_header(message)

        with open(key, 'rb') as private:
            try:
                privateKey = RSA.importKey(private.read())
            except:
                keyType = ['n', 'e', 'd', 'p', 'q']
                private.seek(0, 0)
                for c in range(len(keyType)):
                    keyType[c] = long(private.readline().split('=')[1].strip(), 16)
                #print keydata[c]
                privateKey = RSA.construct(tuple(keyType))

        if privateKey is not None and hash is not None:
            sigFileName = os.path.abspath(bin)
            sigFileName = sigFileName[:sigFileName.rindex('.')] + '.sig' + sigFileName[sigFileName.rindex('.'):]
            with open(sigFileName, 'wb') as sig:
                sig.write(message + PKCS1_v1_5.new(privateKey).sign(SHA256.new(hash)))

            #modulusFile = os.path.abspath(sys.argv[1])[:os.path.abspath(sys.argv[1]).rindex('/')] + '/modulus.bin'
            #print 'modulus: %s' % modulusFile
            #if False == os.path.exists(modulusFile):
            #    with open(modulusFile, 'wb') as modulus:
            #        modulus.write(long_to_bytes(privateKey.n))
    except BaseException as e:
        print str(e)
        pass
def do_encrypt(cipherfile, binfile):
    cipher = None
    plaintext = None
    encryptor = None
    
    try:
        with open(cipherfile, 'rb') as key:
            cipher = key.read()
        with open(binfile, 'rb') as text:
            plaintext = text.read()
    
        if 0 < len(cipher) and  0 < len(plaintext):
            align = len(plaintext) % 16
            if 0 < align:
                print 'total size: %d -> ' % len(plaintext),
                align = 16 - align
                for c in range(align):
                    plaintext += struct.pack('b', 0)
                print '%d' % len(plaintext)
            #print 'AES len %d' % len(cipher)
            encryptor = AES.new(cipher, AES.MODE_ECB)
            if '.' in os.path.basename(binfile):
                binfile = binfile[:binfile.rindex('.')] + '.aes' + binfile[binfile.rindex('.'):]
            else:
                binfile += '.aes'
            print 'out: %s' % binfile
            with open(binfile, 'wb') as out:
                if 0x27051956 == struct.unpack('>I', plaintext[0:4])[0]:
                    print 'uImage header'
                    ciphertext = encryptor.encrypt(plaintext[64:])
                    ciphertext = plaintext[0:64] + ciphertext
                else:
                    ciphertext = encryptor.encrypt(plaintext)
                out.write(ciphertext)
    except BaseException as e:
        print str(e)
        pass
def calc_checksum(plaintext, size):
    checksum = 0
    c = 0
    while size != c:
        checksum += struct.unpack('I', plaintext[c:c+4])[0]
        c += 4
    checksum &= 0xffffffff
    return struct.pack('I', checksum)

def insert_key(binfile, pubkey, cipher):
    plainText = None
    try:
        with open(binfile, 'rb') as bin:
            plainText = bin.read()
        magic, fileSize, cid, auth, checksum, lenN, lenAES = struct.unpack('IH2BI2H', plainText[4:20])
        if 0x4e4C5049 == magic:
            print '** Start insertion **'
            print ' fileSize = %04x' % fileSize
            print ' checksum = %08x' % checksum
            print ' lenN = %d' % lenN
            print ' lenAES = %d' % lenAES

            if pubkey is not None:
                if os.path.isfile(pubkey):
                    with open(pubkey, "rb") as key:
                        lenN = key.read()
                        try:
                            rsa = RSA.importKey(lenN)
                            lenN = long_to_bytes(rsa.n)
                        except:
                            pass
                        plainText += lenN
                        lenN = len(lenN)
            if cipher is not None:
                if os.path.isfile(cipher):
                    with open(cipher, "rb") as aes:
                        lenAES = aes.read()
                        plainText += lenAES
                        lenAES = len(lenAES)
            fileSize = len(plainText)
            print '** End insertion **'
            print ' fileSize = %04x' % fileSize
            plainText = plainText[0:8] + struct.pack('H2BI2H', fileSize, cid, auth, checksum, lenN, lenAES) + plainText[20:]
            checksum = calc_checksum(plainText[16:], len(plainText[16:]))
            checksum = struct.pack('2H', struct.unpack('2H', checksum)[0], fileSize)
            print ' checksum = %08x' % struct.unpack('I', checksum)
            plainText = plainText[:12] + checksum + plainText[16:]
            print ' lenN = %d' % lenN
            print ' lenAES = %d' % lenAES
            if '.' in binfile:
                binfile = binfile[:binfile.rindex('.')] + '.key' + binfile[binfile.rindex('.'):]
            else:
                binfile += '.cipher'
            print binfile
            with open(binfile, 'wb') as out:
                out.write(plainText)
        else:
            print "Not a IPL_CUSTK image"
    except:
        print 'No input file'
        pass
def main():
    binfile = None
    cipherfile = None
    keyfile = None
    ipl = None
    ipl_cust = None
    encrypt = False
    insert = False
    sign = False
    merge = False

    try:
        opts, args = getopt.getopt(sys.argv[1:], 'hf:',['aes=', 'rsa=', 'help', 'encrypt', 'insert', 'sign', 'merge', 'ipl=', 'iplc=']) 
        for o, a in opts:
            if o == "-f":
                binfile = os.path.abspath(a)
                print 'image: %s' % binfile
            elif o == '-h' or o == '--help':
                usage()
            elif o == '--encrypt':
                encrypt = True
            elif o == '--insert':
                insert = True
            elif o == '--sign':
                sign = True
            elif o == '--merge':
                merge = True
            elif o == '--aes':
                cipherfile = os.path.abspath(a)
                print 'aes: %s' % cipherfile
            elif o == "--rsa":
                keyfile = os.path.abspath(a)
                print 'rsa: %s' % keyfile
            elif o == "--ipl":
                ipl = os.path.abspath(a)
                print 'IPL: %s' % ipl
            elif o == "--iplc":
                ipl_cust = os.path.abspath(a)
                print 'IPL_CUST: %s' % ipl_cust                
            else:
                assert False, "unhandled option %s" % o

        if 3 > len(sys.argv):
            usage()

        if True == encrypt:
            do_encrypt(cipherfile, binfile)
        elif True == insert:
            insert_key(binfile, keyfile, cipherfile)
        elif True == sign:
            do_sign(keyfile, binfile)
        elif True == merge:
            do_merge(ipl, ipl_cust)
        else:
            usage()
    except getopt.GetoptError as err:
        # print help information and exit:
        print(err) # will print something like "option -a not recognized"
        usage()

if __name__ == '__main__':
    main()