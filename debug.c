
Id cl_debug() { 
}

// DEBUG
Id __ds(const char *comment, Id va) {
  char b[CL_CELL_SIZE * 2];
  snprintf(b, 1023, "%s%s%s", comment, strlen(comment) > 0 ? " " : "", 
      cl_type_to_i_cp(CL_TYPE(va)));
  if (!cl_is_string(va) && CL_TYPE(va) != CL_TYPE_ARRAY)  {
    snprintf(b + strlen(b), 1023, "%s:", cl_type_to_cp(CL_TYPE(va)));
    if (!cl_is_number(va)) snprintf(b + strlen(b), 1023, "<0x%X> ", CL_ADR(va));
  }
  Id s = cl_to_string(va);
  snprintf(b + strlen(b), CL_CELL_SIZE, "%s%s", cl_string_ptr(s), 
      CL_TYPE(va) == CL_TYPE_STRING ? "'" : "");
  return S(b);
}

Id __d(const char *where, int l, const char *comment, Id va) {
    printf("[%s:%d] %s\n", where, l, cl_string_ptr(__ds(comment, va))); return va; }

