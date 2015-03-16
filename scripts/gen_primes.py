#!/usr/bin/python

import sys

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
