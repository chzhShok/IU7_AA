from constants import (
    EXIT,
    ALL_COMB,
    ANT_ALG,
    ALG_ALL,
    PARAMETRIC,
    TEST,
    UPDATE_DATA,
    SHOW_DATA,
    MESSAGE,
)
from test import parametrization, test_time
from utils import (
    parse_brute_force,
    parse_ant,
    parse_all,
    update_file,
    print_matrix,
)


def menu() -> None:
    option = -1
    while option != EXIT:
        try:
            option = int(input(MESSAGE))
        except Exception:
            option = -1

        if option == ALL_COMB:
            parse_brute_force()
        elif option == ANT_ALG:
            parse_ant()
        elif option == ALG_ALL:
            parse_all()
        elif option == PARAMETRIC:
            parametrization()
        elif option == TEST:
            test_time()
        elif option == UPDATE_DATA:
            update_file()
        elif option == SHOW_DATA:
            print_matrix()
        elif option == EXIT:
            print("Выход из программы")
        else:
            print("Неверный пункт меню, повторите ввод\n")
