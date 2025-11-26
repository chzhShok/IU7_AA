pub(crate) fn count_until_2(slice: &[i32]) -> usize {
    let mut count = 0;
    for &x in slice {
        if x == 2 {
            break;
        }
        count += (x == 1) as usize;
    }
    count
}

fn main() {

}