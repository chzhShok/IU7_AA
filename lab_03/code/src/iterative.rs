pub(crate) fn count_until_2(slice: &[i32]) -> usize {
    let mut count = 0;
    for &x in slice {
        if x == 2 {
            break;
        }
        if x == 1 {
            count += 1;
        }
    }
    count
}
