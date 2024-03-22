# TAI_labwork-1
## Developed by:
- Pompeu Costa, 103294
- Rafael Pinto, 103379
- Ricardo Antunes, 98275

## How to build the program
```bash
g++ -O3 -o cpm cpm.cpp
```

## Parameters
The program accepts 4 optional parameters and 1 mandatory.

### Mandatory
The only mandatory parameter is the input file, which the program is to "compress".

### Optional
The 4 optional parameters are:
| Parameter 	|            Explanation            	| Default Value 	|
|:---------:	|:---------------------------------:	|:-------------:	|
|     k     	|     Sets the size of the kmers    	|       11      	|
|     s     	|  Enables the output of statistics 	|     false     	|
|     a     	| Sets the value of alpha/smoothing 	|       1       	|
|     t     	|         Sets the threshold        	|      0.8      	|

It's important to note that:
- k must be a positive integer
- a must be a decimal value greater than 0
- t must be between 0 and 1 -> [0..1]