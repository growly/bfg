namespace bfg {

// C++ modulo is more 'remainder' than 'modulo' because of how negative numbers
// are handled:
//    mod(-3, 5) = 2
//    rem(-3, 5) = -3 (since -3 / 5 = 0)
// So we have to do this:
inline int64_t modulo(int64_t a, int64_t b) {
  int64_t remainder = a % b;
  return remainder < 0 ? remainder + b : remainder;
}

}   // namespace bfg
