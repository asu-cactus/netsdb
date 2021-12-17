import numpy as np
import itertools

output = np.load('tensor_list.npy', allow_pickle=True).item()
output.keys()
#print(output['list_of_tensors'])

list = [{}, {}, {}, {}, {}]


for num in range(5):
    a0=set(output['list_of_tensors'][0+num*5])
    a1=set(output['list_of_tensors'][1+num*5])
    a2=set(output['list_of_tensors'][2+num*5])
    a3=set(output['list_of_tensors'][3+num*5])
    a4=set(output['list_of_tensors'][4+num*5])
    list[num]=a0.union(a1, a2, a3, a4)
    print(list[num])

#index = [0, 1, 2, 3, 4]
#for L in range(0, len(index)+1):
#    for subset in itertools.combinations(index, L):
#        print(subset)

print("Shared by all tensors:")
l01234=list[0].intersection(list[1], list[2], list[3], list[4])
print(l01234)

print("Shared by 0, 1, 2, 3:")
l0123=list[0].intersection(list[1], list[2], list[3])-l01234
print(l0123)

print("Shared by 0, 1, 2, 4:")
l0124=list[0].intersection(list[1], list[2], list[4])-l01234
print(l0124)

print("Shared by 0, 1, 3, 4:")
l0134=list[0].intersection(list[1], list[3], list[4])-l01234
print(l0134)

print("Shared by 0, 2, 3, 4:")
l0234=list[0].intersection(list[2], list[3], list[4])-l01234
print(l0234)

print("Shared by 1, 2, 3, 4:")
l1234=list[1].intersection(list[2], list[3], list[4])-l01234
print(l1234)

print("Shared by 0, 1, 2:")
l012=list[0].intersection(list[1], list[2])-l0123-l0124-l01234
print(l012)

print("Shared by 0, 1, 3:")
l013=list[0].intersection(list[1], list[3])-l0123-l0134-l01234
print(l013)

print("Shared by 0, 1, 4:")
l014=list[0].intersection(list[1], list[4])-l0124-l0134-l01234
print(l014)

print("Shared by 0, 2, 3:")
l023=list[0].intersection(list[2], list[3])-l0123-l0234-l01234
print(l023)

print("Shared by 0, 2, 4:")
l024=list[0].intersection(list[2], list[4])-l0124-l0234-l01234
print(l024)

print("Shared by 0, 3, 4:")
l034=list[0].intersection(list[3], list[4])-l0134-l0234-l01234
print(l034)

print("Shared by 1, 2, 3:")
l123=list[1].intersection(list[2], list[3])-l0123-l1234-l01234
print(l123)

print("Shared by 1, 2, 4:")
l124=list[1].intersection(list[2], list[4])-l0124-l1234-l01234
print(l124)

print("Shared by 1, 3, 4:")
l134=list[1].intersection(list[3], list[4])-l0134-l1234-l01234
print(l134)

print("Shared by 2, 3, 4:")
l234=list[2].intersection(list[3], list[4])-l0234-l1234-l01234
print(l234)

print("Shared by 0, 1:")
l01=list[0].intersection(list[1])-l012-l013-l014-l0123-l0124-l0134-l01234
print(l01)

print("Shared by 0, 2:")
l02=list[0].intersection(list[2])-l012-l023-l024-l0123-l0124-l0234-l01234
print(l02)

print("Shared by 0, 3:")
l03=list[0].intersection(list[3])-l013-l023-l034-l0123-l0234-l0234-l01234
print(l03)

print("Shared by 0, 4:")
l04=list[0].intersection(list[4])-l014-l024-l034-l0124-l0134-l0234-l01234
print(l04)

print("Shared by 1, 2:")
l12=list[1].intersection(list[2])-l012-l123-l124-l0123-l0124-l1234-l01234
print(l12)

print("Shared by 1, 3:")
l13=list[1].intersection(list[3])-l013-l034-l134-l0123-l0134-l1234-l01234
print(l13)

print("Shared by 1, 4:")
l14=list[1].intersection(list[4])-l014-l124-l134-l0124-l0134-l1234-l01234
print(l14)

print("Shared by 2, 3:")
l23=list[2].intersection(list[3])-l023-l123-l234-l0123-l0234-l1234-l01234
print(l23)

print("Shared by 2, 4:")
l24=list[2].intersection(list[4])-l024-l124-l234-l0124-l0234-l1234-l01234
print(l24)

print("Shared by 3, 4:")
l34=list[3].intersection(list[4])-l034-l134-l234-l0134-l0234-l1234-l01234
print(l34)

print("Private to 0:")
l0=list[0]-l01-l02-l03-l04-l012-l013-l014-l023-l024-l034-l0123-l0124-l0134-l0234-l01234
print(l0)

print("Private to 1:")
l1=list[1]-l01-l12-l13-l14-l012-l013-l014-l123-l124-l134-l0123-l0124-l0134-l1234-l01234
print(l1)

print("Private to 2:")
l2=list[2]-l02-l12-l23-l24-l012-l023-l024-l123-l124-l234-l0123-l0124-l0234-l1234-l01234
print(l2)

print("Private to 3:")
l3=list[3]-l03-l13-l23-l34-l013-l023-l034-l123-l134-l234-l0123-l0134-l0234-l1234-l01234
print(l3)

print("Private to 4:")
l4=list[4]-l04-l14-l24-l34-l014-l024-l034-l124-l134-l234-l0124-l0134-l0234-l1234-l01234
print(l4)






