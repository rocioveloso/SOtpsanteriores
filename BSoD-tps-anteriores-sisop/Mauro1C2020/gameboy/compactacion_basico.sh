#!/bin/sh
./gameboy BROKER CAUGHT_POKEMON 1 OK
./gameboy BROKER CAUGHT_POKEMON 2 FAIL

./gameboy BROKER CATCH_POKEMON Pikachu 2 3
./gameboy BROKER CATCH_POKEMON Squirtle 5 2

./gameboy BROKER CATCH_POKEMON Onyx 4 5
killall -s SIGUSR1 broker
./gameboy BROKER SUSCRIPTOR CAUGHT_POKEMON 10
killall -s SIGUSR1 broker

./gameboy BROKER CATCH_POKEMON Vaporeon 4 5
killall -s SIGUSR1 broker