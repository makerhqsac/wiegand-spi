import spidev

spi = spidev.SpiDev()
spi.open(0,0)

spi.max_speed_hz = 500
spi.mode = 0b00

length = spi.readbytes(1)[0];

if length == 0:
    print("No data available from the reader.")
    quit()

id = spi.readbytes(length)
id.reverse()

print("There were %d byte(s) from the reader:" % length)
print(' '.join(map(lambda x: "%02X" % x, id)))

spi.close()
