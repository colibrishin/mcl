def gen_func(name, ret, args, cname, suf, cargs, n):
	print(f'''template<size_t N>
{ret} {name}({args});''')
	retstr = "" if ret == "void" else " return"
	for i in range(3, n+1):
		print(f'extern "C" {ret} {cname}{i}{suf}({args});')
		print(f'template<> {ret} {name}<{i}>({args}) {{{retstr} {cname}{i}{suf}({cargs}); }}')


print('''#pragma once
// this code is generated by python3 src/gen_proto.py
namespace mcl { namespace bint {''')

N = 4
gen_func("addT", "Unit", "Unit *z, const Unit *x, const Unit *y", "mcl_fp_addPre", "L", "z, x, y", N)
gen_func("subT", "Unit", "Unit *z, const Unit *x, const Unit *y", "mcl_fp_subPre", "L", "z, x, y", N)

print('} } // mcl::bint')
