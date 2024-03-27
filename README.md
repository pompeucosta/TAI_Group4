# TAI Group #4
## Developed by:
- Pompeu Costa, 103294
- Rafael Pinto, 103379
- Ricardo Antunes, 98275

# CPM

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

#### Statistics
The *s* optional parameter outputs statistics to a json file. The objective of those is statistics is to be processed by a script (python for example) for analysis.

The statistics outputed are the following:
- Elapsed time (in ms)
- Number of bits generated by the fallback model
- Number of bits generated by the repeat model
- Number of calls of the fallback model (number of iterations in which the fallback model was used)
- Number of calls of the repeat model (number of iterations in which the repeat model was used)
- Value used to represent the repeat model in the models used array
- Value used to represent the fallback model in the models used array
- The models used array (array in which each elements corresponds to the model used in the respective iteration)
- Bits calculated in each iteration

##### Example of outputed file

```
{
  "elapsedTime": 0,
  "fallbackModelBits": 51.48213,
  "fallbackModelCalls": 29,
  "repeatModelBits": 2.16993,
  "repeatModelCalls": 1,
  "repeatIterationValue": 1,
  "fallbackIterationValue": 0,
  "modelsUsed": [
    1,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    1,
    1,
    1,
    1,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    1,
    1,
    1,
    1,
    1,
    1
  ],
  "bitsCalculated": [
    2.16993,
    1.77525,
    1.77525,
    1.77525,
    1.77525,
    1.77525,
    1.77525,
    1.77525,
    1.77525,
    1.77525,
    1.77525,
    1.77525,
    1.77525,
    1.77525,
    1.77525,
    1.77525,
    1.77525,
    1.77525,
    1.77525,
    1.77525,
    1.77525,
    1.77525,
    1.77525,
    1.77525,
    1.77525,
    1.77525,
    1.77525,
    1.77525,
    1.77525,
    1.77525
  ]
}
```

# Mutate

## How to build the program
```bash
g++ -O3 -o mutate mutate.cpp
```

## Parameters
There are only two parameters and both are mandatory. Those parameters are:
- File path of the file to be mutated
- The probability of mutating (value between 0 and 1 -> [0..1])

The program will output the mutated file and will not change the original one.
