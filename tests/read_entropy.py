import array, fcntl, struct, termios, os, time

RNDGETENTCNT = 0x80045200

dev_random = open('/dev/random')
dev_random_fd = dev_random.fileno()

while True:
	buf = array.array('i', [0])
	fcntl.ioctl(dev_random_fd, RNDGETENTCNT, buf, 1)
	print('Entropy count: {}'.format(
		struct.unpack('I', buf.tobytes())[0]
		),
	end='\r'
	)
	time.sleep(1)