pub(crate) fn count_until_2(slice: &[i32]) -> usize {
    if slice.is_empty() {
        0
    } else if slice[0] == 2 {
        0
    } else {
        let this = slice[0] as usize;
        this + count_until_2(&slice[1..])
    }
}
