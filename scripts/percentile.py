# Takes as input a file with integer values on each line. Outputs a csv file with "percentile, value"

import sys
import numpy as np

input_file = sys.argv[1]
op_file = sys.argv[2]

def main():
  values = np.array(open(input_file,'r').readlines(), dtype=int)
  with open (op_file, 'w') as op:
    for i in range(101):
      op.write("%d,%d\n" %(i, int(np.percentile(values,i))))

if __name__ == "__main__":
    main()
