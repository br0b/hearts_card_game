import functools
import operator

from random import randint, shuffle

values = ["2", "3", "4", "5", "6", "7", "8", "9", "10", "J", "Q", "K", "A"]
colors = ["C", "D", "H", "S"]
deck = [v + c for v in values for c in colors]
players = ["N", "E", "S", "W"]
n_game_modes = 1
n_deals = 1

for deal in range(n_deals):
    game_mode = str(randint(1, n_game_modes))
    starting_player = "N"
    shuffle(deck)
    print(game_mode + starting_player)
    for i in range(len(players)):
        print(functools.reduce(operator.add, deck[13 * i:13 * (i + 1)]))
