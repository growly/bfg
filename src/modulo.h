namespace bfg {

// C++ modulo is more 'remainder' than 'modulo' because of how negative numbers
// are handled:
//    mod(-3, 5) = 2
//    rem(-3, 5) = -3 (since -3 / 5 = 0)
// So we have to do this:
template<typename T>
inline T modulo(T a, T b) {
  int64_t remainder = a % b;
  return remainder < 0 ? remainder + b : remainder;
}

}   // namespace bfg
