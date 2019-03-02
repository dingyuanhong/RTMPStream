import base64

# sps='67 64 00 33 ac 1b 1a 80 2f 80 bf a1 00 00 00 01 00 00 00 3c 8f 14 2a a0'
# pps='68 ee 3c b0'

# sps='67 42 80 32 da 00 be 02 fe 94 82 81 01 03 68 50 9a 80'
# pps='68 ce 06 e2'

# sps='67 64 00 32 ac b4 01 7c 05 fd 08 00 00 00 08 00 00 01 e4 78 c1 95'
# pps='68 ef 3c b0'

#sps = '67 64 00 32 ac b4 01 7c 05 fd 08 00 00 03 00 08 00 00 03 01 e4 78 c1 95'
#pps = '68 ef 3c b0'

sps = '67 42 c0 28 da 01 e0 08 9f 96 10 00 00 03 00 10 00 00 03 03 c8 f1 83 2a'
pps = '68 ce 3c 80'

def HEX(a):
	if a >= '0' and a <= '9':
		v = ord(a) - ord('0');
		# print(v);
	else:
		v = ord(a) - ord('a') + 10;
		# print(v);
	return v;

def encodeHEX(sps):
	s_sps=''
	for i in range(0, len(sps),3):
		s_sps += chr(HEX(sps[i])*16 + HEX(sps[i+1]));
		
	# for i in s_sps:
		# print('%#x'%ord(i))
		
	a_sps = base64.b64encode(s_sps);
	
	return a_sps;

sps = encodeHEX(sps)
pps = encodeHEX(pps)

print(sps);
print(pps);

with open("sps.txt","w") as f:
	f.write(sps);
with open("pps.txt","w") as f:
	f.write(pps);

# ds_sps = base64.b64decode(a_sps);
# for i in ds_sps:
	# print('%#x'%ord(i))