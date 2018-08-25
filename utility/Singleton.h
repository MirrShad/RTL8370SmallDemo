#ifndef _FW_SINGLETON_H
#define _FW_SINGLETON_H

/************************************************************************/
/*            Meyers Singleton: Release resource before exiting			*/
/************************************************************************/
template <class SingletonClass >
class MeyersSingleton{
public:
	static SingletonClass * Instance(){
		static SingletonClass instance;
		return &instance;
	}

	SingletonClass* operator ->() { return Instance(); }

	const SingletonClass* operator ->() const { return Instance(); }

private:
	MeyersSingleton(){ }

	~MeyersSingleton(){ }
};

/************************************************************************/
/*			Normal Singleton: Not Release resource before exiting		*/
/************************************************************************/
template <class SingletonClass >
class NormalSingleton{
public:
	static SingletonClass * Instance(){

		if( !ins ){
			ins = new SingletonClass;
		}
		return ins;
	}

	SingletonClass* operator ->() { return Instance(); }

	const SingletonClass* operator ->() const { return Instance(); }

private:
	NormalSingleton(){ }

	~NormalSingleton(){ }

	static SingletonClass* ins;
};

template <class SingletonClass >
SingletonClass* NormalSingleton<SingletonClass>::ins = 0;
#endif	// ~ _N_SINGLETON_H
