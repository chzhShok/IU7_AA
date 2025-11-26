use std::io::{self, Write};
use std::time::Instant;

use crate::iterative;
use crate::recursive;

pub(crate) fn measure_with_varying_input() {
    let (min_size, max_size, step, k) = get_parameters();

    println!("\nРазмер\tРекурсия (мкс)\tИтерация (мкс)");
    println!("----------------------------------------");

    for size in (min_size..=max_size).step_by(step) {
        let test_data = generate_test_data(size);

        let start = Instant::now();
        for _ in 0..k {
            let _ = recursive::count_until_2(&test_data);
        }
        let dur_rec = start.elapsed().as_micros() as f64 / k as f64;

        let start = Instant::now();
        for _ in 0..k {
            let _ = iterative::count_until_2(&test_data);
        }
        let dur_iter = start.elapsed().as_micros() as f64 / k as f64;

        println!("{}\t{:.3}\t\t{:.3}", size, dur_rec, dur_iter);
    }
}

fn get_parameters() -> (usize, usize, usize, usize) {
    println!("Введите параметры для замера времени:");

    let min_size = get_number_input("Минимальный размер последовательности: ");
    let max_size = get_number_input("Максимальный размер последовательности: ");
    let step = get_number_input("Шаг изменения размера: ");
    let k = get_number_input("Количество повторений (k >= 100): ");

    (min_size, max_size, step, k.max(100))
}

fn get_number_input(prompt: &str) -> usize {
    loop {
        print!("{}", prompt);
        io::stdout().flush().unwrap();

        let mut input = String::new();
        io::stdin().read_line(&mut input).unwrap();

        match input.trim().parse::<usize>() {
            Ok(n) => return n,
            Err(_) => println!("Пожалуйста, введите целое положительное число"),
        }
    }
}

fn generate_test_data(size: usize) -> Vec<i32> {
    let mut data = Vec::with_capacity(size + 1);

    for _ in 0..size {
        let value = if rand::random::<bool>() { 1 } else { 0 };
        data.push(value);
    }

    data.push(2);

    data
}
