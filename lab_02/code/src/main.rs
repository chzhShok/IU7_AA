use std::io::{self, BufRead, Write};

mod iterative;
mod recursive;
mod timing;

fn main() {
    loop {
        println!("Меню:");
        println!("1: решить рекурсивным алгоритмом");
        println!("2: решить нерекурсивным алгоритмом");
        println!("3: замер процессорного времени выполнения алгоритмов");
        println!("0: выход");
        print!("Выберите пункт: ");
        io::stdout().flush().unwrap();

        let mut choice = String::new();
        io::stdin().read_line(&mut choice).unwrap();

        match choice.trim() {
            "1" => {
                let nums = read_until_2();
                let result = recursive::count_until_2(&nums);
                println!("Рекурсия: {}", result);
            }
            "2" => {
                let nums = read_until_2();
                let result = iterative::count_until_2(&nums);
                println!("Не рекурсия: {}", result);
            }
            "3" => {
                timing::measure_with_varying_input();
            }
            "0" => {
                println!("Выход");
                break;
            }
            _ => println!("Неверный выбор, попробуйте снова"),
        }
    }
}

fn read_until_2() -> Vec<i32> {
    let stdin = io::stdin();
    let mut nums = Vec::new();

    println!("Введите последовательность чисел (0/1, окончание - число 2):");
    for line in stdin.lock().lines() {
        let line = line.unwrap();
        for tok in line.split_whitespace() {
            match tok.parse::<i32>() {
                Ok(n) if n == 0 || n == 1 || n == 2 => {
                    nums.push(n);
                    if n == 2 {
                        return nums;
                    }
                }
                Ok(n) => {
                    eprintln!("Недопустимое число {}. Разрешены только 0, 1, 2", n);
                }
                Err(_) => {
                    eprintln!("Некорректный ввод: '{}'", tok);
                }
            }
        }
    }
    nums
}
