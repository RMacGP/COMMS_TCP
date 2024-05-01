import subprocess
import numpy as np

sample_data = []

for i in range(1000):
    subprocess.run("./client.exe", input=f"http://faraday1.ucd.ie/archive/thesis/phdthesis_federico_milano.pdf\nbmilano{i}.pdf".encode('utf-8'))
    f = open("out_sample_data.txt", "r")
    data = f.readline()
    # print(data)
    # print(data.split(' '))
    data = [ int(x) for x in data.split(' ') if x != '']
    sample_data.append(data)
    # print(data)
    f.close()


means = [ np.mean(i) for i in sample_data ]
ns = [ len(i) for i in sample_data ]


# save_data = open("compiled_data.txt", "w")
# for i in sample_data:
#     save_data.write(", ".join(str(x) for x in i))
save_data = open("sample_means.txt", "w")
save_data.write(", ".join(str(x) for x in means))
save_data.close()
save_ns = open("sample_sizes.txt", "w")
save_ns.write(", ".join(str(x) for x in ns))
print(np.mean(means))
print(np.median(ns))

