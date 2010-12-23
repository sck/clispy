/*
 * Scheme parsing code, port of Norvig's lis.py:
 * <http://norvig.com/lispy.html>
 */

Id cl_tokenize(Id va_s) {
  if (!va_s.s) return clNil;
  return cl_string_split(
      cl_string_replace(cl_string_replace(va_s, S("("), S(" ( ")),
      S(")"), S(" ) ")));
}

Id cl_atom(Id token) {
  CL_ACQUIRE_STR_D(dt, token, clNil);
  char *ep;
  long l = strtol(dt.s, &ep, 10);
  if (ep && *ep == '\0') return cl_int((int)l);
  float f = strtof(dt.s, &ep);
  if (ep && *ep == '\0') return cl_float(f);
  return cl_intern(token);
}

Id cl_read_from(Id tokens) {
  cl_reset_errors();
  if (!tokens.s) return clNil;
  if (cl_ary_len(tokens) == 0) 
      return cl_handle_error_with_err_string_nh(__FUNCTION__, 
          "unexpected EOF while reading");
  Id token = cl_ary_unshift(tokens);
  if (cl_string_equals_cp_i(token, "(")) {
    Id l = cl_ary_new();
    while (!cl_string_equals_cp_i(ca_f(tokens), ")")) {
        cl_ary_push(l, cl_read_from(tokens)); CE(break) }
    cl_ary_unshift(tokens);
    return l;
  } else if (cl_string_equals_cp_i(token, ")")) {
    return cl_handle_error_with_err_string_nh(__FUNCTION__, 
        "unexcpeted )");
  } else return cl_atom(token);
  return clNil;
}

Id cl_parse(Id va_s) { return cl_read_from(cl_tokenize(va_s)); }

Id cl_eval(Id x, Id env) {
  if (!x.s) return clNil;
  env = (env.s ? env : cl_global_env);
  if (CL_TYPE(x) == CL_TYPE_SYMBOL) {
    return cl_env_find(env, x);
  } else if (CL_TYPE(x) != CL_TYPE_ARRAY) {
    return x; // constant literal
  } 
  Id x0 = ca_f(x), exp, val= clNil, var;
  if (cl_string_equals_cp_i(x0, "quote")) {
    return ca_s(x);
  } else if (cl_string_equals_cp_i(x0, "if")) { // (if test conseq alt)
    Id test = ca_s(x), conseq = ca_th(x), alt = ca_fth(x);
    return cnil2(cl_eval(test, env)).s ? cl_eval(conseq, env) : cl_eval(alt, env);
  } else if (cl_string_equals_cp_i(x0, "set!")) { // (set! var exp)
    var = ca_s(x), exp = ca_th(x);
    cl_env_find_and_set(env, var, cl_eval(exp, env));
  } else if (cl_string_equals_cp_i(x0, "define")) { // (define var exp)
    var = ca_s(x), exp = ca_th(x);
    cl_ht_set(env, var, cl_eval(exp, env));
  } else if (cl_string_equals_cp_i(x0, "lambda")) { //(lambda (var*) exp)
    Id l = cl_ary_new(); cl_ary_push(l, ca_s(x)); cl_ary_push(l, ca_th(x));
    cl_ary_push(l, env);
    return l; 
  } else if (cl_string_equals_cp_i(x0, "begin")) {  // (begin exp*)
    int i = 1;
    while ((exp = cl_ary_iterate(x, &i)).s) val = cl_eval(exp, env);
    return val;
  } else {  // (proc exp*)
    Id vars = cl_ary_new(), v;
    int i = 1;
    while ((v = cl_ary_iterate(x, &i)).s) cl_ary_push(vars, cl_eval(v, env));
    Id lambda = cl_env_find(env, x0);
    if (!lambda.s) { return cl_handle_error_with_err_string(__FUNCTION__, 
            "Unknown proc", cl_string_ptr(x0)); }
    if (cl_is_type_i(lambda, CL_TYPE_CFUNC)) return cl_call(lambda, vars);
    Id vdecl = cl_ary_index(lambda, 0);
    if (cl_ary_len(vdecl) != cl_ary_len(vars)) 
        return cl_handle_error_with_err_string(__FUNCTION__, 
            "parameter count mismatch!", cl_string_ptr(x0));
    Id e = cl_env_new(cl_ary_index(lambda, 2)), p;
    i = 0;
    while ((p = cl_ary_iterate(vdecl, &i)).s)  {
        cl_ht_set(e, p, cl_ary_index(vars, i - 1));
    }
    return cl_eval(cl_ary_index(lambda, 1), e);
  }
  return clNil;
}


Id  __try_convert_to_floats(Id x) {
  Id a = cl_ary_new(), n;
  int i = 0; 
  while ((n = cl_ary_iterate(x, &i)).s) {
    if (!cl_is_number(n)) return clNil;
    cl_ary_push(a, CL_TYPE(n) == CL_TYPE_INT ? cl_float(CL_INT(n)) : n);
  }
  return a;
}

Id  __try_convert_to_ints(Id x) {
  Id a = cl_ary_new(), n0, n;
  int i = 0; 
  while ((n0 = cl_ary_iterate(x, &i)).s) {
    n = cn(n0);
    if (!cl_is_number(n)) return clNil;
    cl_ary_push(a, n);
  }
  return a;
}

#define ON_I \
  int t = (cl_ary_contains_only_type_i(x, CL_TYPE_INT) ? 1 : \
      (cl_ary_contains_only_type_i(x, CL_TYPE_FLOAT) ? 2 : 0)); \
  if (t == 0) { \
      Id try = __try_convert_to_ints(x);  \
      if (try.s) { t = 1; x = try; }} \
  if (t == 0) { \
      Id try = __try_convert_to_floats(x);  \
      if (try.s) { t = 2; x = try; }} \
  int ai = CL_INT(ca_f(x)); int bi = CL_INT(ca_s(x)); \
  float af = CL_FLOAT(ca_f(x)); float bf = CL_FLOAT(ca_s(x)); \
  Id r = clNil; \
  if (t == 1) { 
#define ON_F ; } else if (t == 2) {
#define R  ; } return r;

Id cl_to_string(Id exp);

Id cl_add(Id x) { ON_I r = cl_int(ai + bi) ON_F r = cl_float(af + bf) R }
Id cl_sub(Id x) { ON_I r = cl_int(ai - bi) ON_F r = cl_float(af - bf) R }
Id cl_mul(Id x) { ON_I r = cl_int(ai * bi) ON_F r = cl_float(af * bf) R }
Id cl_div(Id x) { ON_I r = cl_int(ai / bi) ON_F r = cl_float(af / bf) R }
Id cl_not(Id x) { ON_I r = cl_int(ai ^ bi) R }
Id cl_gt(Id x) { ON_I r = cb(ai > bi) ON_F r = cb(af > bf) R }
Id cl_lt(Id x) { ON_I r = cb(ai < bi) ON_F r = cb(af < bf) R }
Id cl_ge(Id x) { ON_I r = cb(ai >= bi) ON_F r = cb(af >= bf) R }
Id cl_le(Id x) { ON_I r = cb(ai <= bi) ON_F r = cb(af <= bf) R }
Id cl_eq(Id x) { return cb(cl_equals_i(ca_f(x), ca_s(x))); }
Id cl_length(Id x) { return cl_int(cl_ary_len(x)); }
Id cl_cons(Id x) { return cl_ary_new_join(ca_f(x), ca_s(x)); }
Id cl_car(Id x) { return ca_f(ca_f(x)); }
Id cl_cdr(Id x) { Id c = cl_ary_clone(ca_f(x)); cl_ary_unshift(c); return c; }
Id cl_list(Id x) { return x; }
Id cl_is_list(Id x) { return cb(cl_is_type_i(x, CL_TYPE_ARRAY)); }
Id cl_is_null(Id x) { return cb(cnil(x)); }
Id cl_is_symbol(Id x) { return cb(cl_is_type_i(x, CL_TYPE_SYMBOL)); }
Id cl_display(Id x) { printf("%s", cl_string_ptr(cl_ary_join_by_s(
    cl_ary_map(x, cl_to_string), S(" ")))); return clNil;}
Id cl_newline(Id x) { printf("\n"); return clNil;}

char *cl_std_n[] = {"+", "-", "*", "/", "not", ">", "<", ">=", "<=", "=",
    "equal?", "eq?", "length", "cons", "car", "cdr", "list", "list?", 
    "null?", "symbol?", "display", "newline", 0};
Id (*cl_std_f[])(Id) = {cl_add, cl_sub, cl_mul, cl_div, cl_not, cl_gt, cl_lt, cl_ge, 
    cl_le, cl_eq, cl_eq, cl_eq, cl_length, cl_cons, cl_car, cl_cdr,
    cl_list, cl_is_list, cl_is_null, cl_is_symbol, cl_display,
    cl_newline, 0};

void cl_add_globals(Id env) {
  int i = 0;
  while (cl_std_n[i] != 0) { cl_define_func(cl_std_n[i], cl_std_f[i], env); i++; }
}

Id cl_to_string(Id exp) {
  if (CL_TYPE(exp) == CL_TYPE_BOOL) { return S(exp.s ? "true" : "null"); }
  if (cl_is_number(exp)) return cl_string_new_number(exp);
  if (CL_TYPE(exp) == CL_TYPE_CFUNC) return S("CFUNC");
  if (CL_TYPE(exp) != CL_TYPE_ARRAY) return exp;
  Id s = S("(");
  cl_string_append(s, cl_ary_join_by_s(cl_ary_map(exp, cl_to_string), S(" ")));
  return cl_string_append(s, S(")"));
}

void cl_repl() {
  while (1) {
    Id val = cl_eval(cl_parse(cl_input("clispy> ")), cl_global_env);
    if (feof(fin)) return;
    if (cl_interactive) printf("-> %s\n", cl_string_ptr(cl_to_string(val)));
    cl_garbage_collect();
  }
}
