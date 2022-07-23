from bleak import BleakClient
import asyncio
import aiofile as aiof
import struct
from datetime import datetime
import matplotlib.pyplot as plt
import matplotlib.animation as animation

ADDRESS = '0c:b8:15:f5:1a:be'
X_UUID = 'beb5483e-36e1-4688-b7f5-ea07361b26a8'
Y_UUID = '02f38517-3c2e-45b3-a7dd-ac426a2b37d6'
Z_UUID = '4de8baa7-4555-48ea-a9f8-8184c023b7fd'
ACCEL_MAG_UUID = '1ce24875-932e-4049-b99f-948746f375e4'

#fig, axs = plt.subplots(3)
ts = list(range(0, 200))
xs = [0] * len(ts)
ys = [0] * len(ts)
zs = [0] * len(ts)
readings = [1] * 3

#line_x, = axs.plot(ts, xs)
#line_y, = axs[1].plot(ts, ys)
#line_z, = axs[2].plot(ts, zs)
#lines = [line_x]#, line_y, line_z]

def animate(i, xs, ys, zs):
    xs.append(readings[0])
    #ys.append(readings[1])
    #zs.append(readings[2])

    xs = xs[-len(ts):]
    #ys = ys[-len(ts):]
    #zs = zs[-len(ts):]

    lines[0].set_ydata(xs)
    #lines[1].set_ydata(ys)
    #lines[2].set_ydata(zs)

    return lines,

async def main(address):
    async with aiof.async_open('data.txt', "w") as out:
        await out.write('')
    data = []
    async with BleakClient(address) as client:
        async with aiof.async_open('data.txt', 'a') as out:
            print(f'Connected: {client.is_connected}')
            while True:
                global readings
                x_val = await client.read_gatt_char(Y_UUID)
                #y_val = await client.read_gatt_char(Y_UUID)
                #z_val = await client.read_gatt_char(Z_UUID)

                readings[0] = struct.unpack('f', x_val)[0] * -1
                #readings[1] = struct.unpack('f', y_val)[0] * -1
                #readings[2] = struct.unpack('f', z_val)[0] * -1
                await out.write(f'{datetime.now().strftime("%H%M%S%f")},{readings[0]}\n')

                # print(readings)

                #try:
                    #plt.draw()
                    #plt.pause(0.0001)
                    #await asyncio.sleep(0.0001)
                #except KeyboardInterrupt:
                #    break

#ani = animation.FuncAnimation(fig, animate, fargs=(xs, ys, zs,), interval=1)
#for i in range(3):
#    axs[i].set_ylim(-2, 2)
#plt.ion()
#plt.show()
asyncio.run(main(ADDRESS))