from bleak import BleakClient
import asyncio

ADDRESS = '0c:b8:15:f5:1a:be'
UUID = 'beb5483e-36e1-4688-b7f5-ea07361b26a8'

async def main(address):
    async with BleakClient(address) as client:
        print(f'Connected: {client.is_connected}')
        while True:
            model_number = await client.read_gatt_char(UUID)
            print("Model Number: {0}".format("".join(map(chr, model_number))))
            await asyncio.sleep(0.1)

asyncio.run(main(ADDRESS))