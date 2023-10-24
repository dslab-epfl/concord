import matplotlib.pyplot as plt

# Load data from the file
def read_data(file_name):
    with open(file_name, 'r') as file:
        data = file.read()
    return data.strip().split('\n')

# Read overhead data from the file
data_lines = read_data('fig2.txt')

# Parse the overhead data
quantums = []
ipi_overhead = []
concord_overhead = []
rdtsc_overhead = []
no_instrument_overhead = []

for line in data_lines:
    parts = line.split()
    quantum = int(parts[1])
    overhead_value = int(parts[2])
    if parts[0] == "0":
        ipi_overhead.append(overhead_value)
    elif parts[0] == "1":
        concord_overhead.append(overhead_value)
    elif parts[0] == "2":
        rdtsc_overhead.append(overhead_value)
    else:
        quantums.append(quantum)
        no_instrument_overhead.append(overhead_value)

# Calculate the overhead for each method (ipi, concord, rdtsc) with respect to no_instrument
ipi_overhead_ratio = [ ((no_instrument - ipi) / no_instrument) for ipi, no_instrument in zip(ipi_overhead, no_instrument_overhead)]
concord_overhead_ratio = [((no_instrument - concord) / no_instrument) for concord, no_instrument in zip(concord_overhead, no_instrument_overhead)]
rdtsc_overhead_ratio = [((no_instrument - rdtsc) / no_instrument) for rdtsc, no_instrument in zip(rdtsc_overhead, no_instrument_overhead)]

# Plot the overhead values using a line chart
plt.figure(figsize=(10, 6))
plt.plot(quantums, ipi_overhead_ratio, label='ipi')
plt.plot(quantums, concord_overhead_ratio, label='concord')
plt.plot(quantums, rdtsc_overhead_ratio, label='rdtsc')
plt.xlabel('Quantum')
plt.ylabel('Overhead Ratio')
plt.title('Overhead Comparison')
plt.legend()
plt.grid(True)

# Save the plot to a file
plt.savefig('fig2.png', bbox_inches='tight')