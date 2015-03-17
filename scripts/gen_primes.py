#!/usr/bin/python

import sys
import multiprocessing

def test_primality(candidate, sieve):
    if (len(sieve) == 0):
        return True

    sieve_index = 0;
    cur_prime = sieve[sieve_index]
    while ((cur_prime * cur_prime <= candidate) and (sieve_index < len(sieve))):
        if ((candidate % cur_prime) == 0):
            return False
        sieve_index += 1
        cur_prime = sieve[sieve_index]

    return True

if __name__ == '__main__':
    if (len(sys.argv) < 3):
        print ('Usage: %s <max-prime-value> <output-file>' % sys.argv[0])
        sys.exit(1)

    num_primes = 0
    max_value = long(sys.argv[1])

    sieve = []

    candidate = long(2)
    while (candidate <= max_value):
        if (test_primality(candidate, sieve)):
            num_primes += 1;
            sieve.append(candidate)

        if (candidate == 2):
                candidate = 3
        else:
            candidate += 2

    print('Found %d primes in all!' % num_primes)

    with open(sys.argv[2], 'w') as output_file:
        output_file.write('static u64 precomputed_prime_list_ = {\n')
        num_on_line = 0

        for i in range(0, len(sieve)):
            if (num_on_line % 8 == 0):
                output_file.write('    ');
            output_file.write('%d' % sieve[i])
            num_on_line += 1
            if (i != len(sieve) - 1):
                output_file.write(',');
                if (num_on_line % 8 == 0):
                    output_file.write('\n')
                else:
                    output_file.write(' ');
        output_file.write('\n};\n\n')
        output_file.write('static constexpr u64 num_precomputed_primes_ = %d;\n' % len(sieve))
        output_file.write('static constexpr u64 max_precomputed_prime_ = %d;\n' % sieve[len(sieve) - 1])
