#python program for Bubble Sort Algorithm
import numpy as np
#define MAX 10
list = np.array([1,8,4,6,0,3,5,2,7,9])
def display():
    print("[", end="")
    #navigate through all items
    for i in range(len(list)):
        print(list[i], end=" ")
    print("]")
def bubbleSort():
    temp = 0
    swapped = False
    
    #loop through all numbers

    for i in range(len(list)-1):
        swapped = False
        # loop through numbers falling ahead 
        for j in range(len(list)-1-i):
            print("     Items compared: [", list[j], ",", list[j+1], "]", end="")
         #check if next number is lesser than current no
         #swap the numbers. 
         #(Bubble up the highest number)
            if(list[j] > list[j+1]):
                temp = list[j]
                list[j] = list[j+1]
                list[j+1] = temp
                swapped = True
                print(" => swapped [", list[j], ",", list[j+1], "]")
            else:
                print(" => not swapped")
        
        #if no number was swapped that means 
        #array is sorted now, break the loop.
        if(not swapped):
            break
        print("Iteration", (i+1), "#: ", end="")
        display()

print("Input Array: ", end="")
display()
bubbleSort()
print("\nOutput Array: ", end="")
display()
