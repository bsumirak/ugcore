

#include "../../ug_bridge.h"
#include "common/common.h"
#include "lib_discretization/lib_discretization.h"
#include "../../../ug_script/ug_script.h"

namespace ug
{
namespace bridge
{

template <int dim>
class ConstUserMatrix
{
	public:
		ConstUserMatrix() {set_diag_tensor(1.0);}

		void set_diag_tensor(number val)
		{
			for(size_t i = 0; i < dim; ++i){
				for(size_t j = 0; j < dim; ++j){
					m_Tensor[i][j] = 0;
				}
				m_Tensor[i][i] = val;
			}
		}

		void set_all_entries(number val)
		{
			for(size_t i = 0; i < dim; ++i){
				for(size_t j = 0; j < dim; ++j){
					m_Tensor[i][j] = val;
				}
			}
		}

		void set_entry(size_t i, size_t j, number val)
		{
			m_Tensor[i][j] = val;
		}

		void print() const
		{
			UG_LOG("ConstUserMatrix:\n" << m_Tensor << "\n");
		}

		void operator() (MathMatrix<dim, dim>& D, const MathVector<dim>& x, number time = 0.0)
		{
			D = m_Tensor;
		}

	protected:
		MathMatrix<dim, dim> m_Tensor;
};

template <int dim>
class LuaUserMatrix
{
	public:
		LuaUserMatrix()
		{
			m_L = ug::script::GetDefaultLuaState();
		}

		void set_lua_callback(const char* luaCallback)
		{
		//	Todo: Work on function pointer directly
			m_callbackName = luaCallback;
		}

		void operator() (MathMatrix<dim, dim>& D, const MathVector<dim>& x, number time = 0.0)
		{
			lua_getglobal(m_L, m_callbackName);
			for(size_t i = 0; i < dim; ++i)
				lua_pushnumber(m_L, x[i]);
			lua_pushnumber(m_L, time);

			size_t matSize = dim*dim;

			if(lua_pcall(m_L, dim + 1, matSize, 0) != 0)
			{
				UG_LOG("error running diffusion callback " << m_callbackName << ": "
								<< lua_tostring(m_L, -1));
				throw(int(0));
			}

			int counter = -1;
			for(size_t i = 0; i < dim; ++i){
				for(size_t j = 0; j < dim; ++j){
					D[i][j] = luaL_checknumber(m_L, counter--);
				}
			}

			lua_pop(m_L, matSize);
		}

	protected:
		const char* m_callbackName;
		lua_State*	m_L;
};


template <int dim, typename TUserMatrix>
class UserMatrixProvider : public IUserMatrixProvider<dim>
{
	public:
	//	Functor Type
		typedef typename IUserMatrixProvider<dim>::functor_type functor_type;

	//	return functor
		virtual functor_type get_functor() const {return m_UserMatrix;}

	//	set user matrix
		void set_functor(const TUserMatrix& userMatrix) {m_UserMatrix = userMatrix;}

	protected:
		TUserMatrix	m_UserMatrix;
};

template <int dim>
void RegisterUserMatrix(Registry& reg, const char* parentGroup)
{
	std::string grp = std::string(parentGroup);


//	Functor
	{
	//	ConstUserMatrix
		{
			typedef ConstUserMatrix<dim> T;
			stringstream ss; ss << "ConstUserMatrix" << dim << "d";
			reg.add_class_<T>(ss.str().c_str(), grp.c_str())
				.add_constructor()
				.add_method("set_diag_tensor", &T::set_diag_tensor)
				.add_method("set_all_entries", &T::set_all_entries)
				.add_method("set_entry", &T::set_entry)
				.add_method("print", &T::print);
		}

	//	LuaUserMatrix
		{
			typedef LuaUserMatrix<dim> T;
			stringstream ss; ss << "LuaUserMatrix" << dim << "d";
			reg.add_class_<T>(ss.str().c_str(), grp.c_str())
				.add_constructor()
				.add_method("set_lua_callback", &T::set_lua_callback);
		}
	}

//	UserMatrixProvider
	{
	//	Base class
		{
			stringstream ss; ss << "IUserMatrixProvider" << dim << "d";
			reg.add_class_<IUserMatrixProvider<dim> >(ss.str().c_str(), grp.c_str());
		}

	//	Const Matrix Provider
		{
			typedef UserMatrixProvider<dim, ConstUserMatrix<dim> > T;
			stringstream ss; ss << "ConstUserMatrixProvider" << dim << "d";
			reg.add_class_<T, IUserMatrixProvider<dim> >(ss.str().c_str(), grp.c_str())
				.add_constructor()
				.add_method("set_functor", &T::set_functor);
		}

	//	Lua Matrix
		{
			typedef UserMatrixProvider<dim, LuaUserMatrix<dim> > T;
			stringstream ss; ss << "LuaUserMatrixProvider" << dim << "d";
			reg.add_class_<T, IUserMatrixProvider<dim> >(ss.str().c_str(), grp.c_str())
				.add_constructor()
				.add_method("set_functor", &T::set_functor);
		}
	}
}

void RegisterUserMatrix(Registry& reg, const char* parentGroup)
{
	RegisterUserMatrix<1>(reg, parentGroup);
	RegisterUserMatrix<2>(reg, parentGroup);
	RegisterUserMatrix<3>(reg, parentGroup);
}


} // end namespace
} // end namepace
