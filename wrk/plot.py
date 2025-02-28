import struct
import matplotlib.pyplot as plt

def read_GDP_from_binary(filename):
    with open(filename, "rb") as file:
        # Read the size of the array first
        size_data = file.read(8)  # size_t is usually 8 bytes
        size = struct.unpack("Q", size_data)[0]  # Read the number of elements

        # Read the GDP values
        GDPs = struct.unpack(f"{size}d", file.read(size * 8))  # 'd' = double (8 bytes)

    return GDPs

def plot_GDP(GDPs):
    plt.figure(figsize=(8, 5))
    plt.plot(GDPs, linestyle="-", color="b", label="GDP Values")
    plt.xlabel("Cycles")
    plt.ylabel("GDP")
    plt.title("GDP Values from Binary File")
    plt.legend()
    plt.grid()
    plt.show()

if __name__ == "__main__":
    gdp_data = read_GDP_from_binary("GDP_data.bin");
    #print("Read GDP Data:", gdp_data)
    plot_GDP(gdp_data)

