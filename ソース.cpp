#include <iostream>
#include <cstdint>
#include <vector>
#include <deque>
#include <tuple>
#include <memory>

#include <iomanip> 

template<class Register=std::intmax_t,class Ops=std::int16_t,class Nim=std::tuple<Ops,  Register, Register> >
class VirtualCPU {
public:
	typedef Register Register;
	typedef Ops Ops;
	typedef Nim Nim;//nimonic and parametors.

	typedef std::shared_ptr<VirtualCPU> spVirtualCPU;

	bool Initialize(std::size_t RN,std::size_t MemorySize) {
		Clear();
		R.resize(RN);
		Memory.resize(MemorySize);

		return true;
	}

	bool Clear() {
		R.clear();
		Q.clear();
		Stack.clear();
		Memory.clear();

		return true;
	}

	virtual bool Push(const Nim& N) {//netburst architectuer
		Q.push_back(N);
		return true;
	}

//	const Register& Get() {
//		return De;
//	}

	virtual bool Update() = 0;//Nimonic instraction.

	const Register& Registers(std::size_t P)const {
		return R[P];
	}

	std::size_t MemorySize() const{
		return Memory.size();
	}

	Register& operator [](std::size_t I) {
		return Memory[I];
	}
	const Register& operator [](std::size_t I)const {
		return Memory[I];
	}
	std::size_t RegisterSize() const {
		return R.size();
	}
	std::size_t StackSize() {
		return Stack.size();
	}
	std::size_t QueueSize() {
		return Q.size();
	}
	bool IsEnd() const {
		return ToEnd;
	}

protected:
	
	bool ToEnd = false;
//	Register AR{ 0 };//result register.
	std::deque<Nim> Q;
	std::vector<Register> R;
	std::vector<Register> Memory;
	std::vector<std::vector<Register>> Stack;
};

enum class Ops : std::uint8_t {
	Null = 0,
	And,
	Or,
	Not,
	Xor,
	Nand,
	Nor,
	Equal,
	Add,
	Sub,
	Mul,
	Div,
	Mod,
	Hit,//one of Bit Test
	RWin,//R is greater.
	LWin,//L is greater.
	BWin,//Both Win.
	LShift,//bit shift left.
	RShift,//bit shift right.
	Set,//[RegisterPos,Value]//initialize Register.
	Label,//Label for Jamp.
	JNZ,//jamp if non zero.
	JIZ,//jamp if zero
	Test,//BitTest by tow value. answer is yet unknown.
	Int,//Special Service.
	Load,//from Memory.
	Store,//To Memory
	Move,//register to register.
//	EEnd,//Terminater of Enum.
};

class TestCPU :public VirtualCPU<std::int16_t, Ops>
{
public:
	enum class IntOps : VirtualCPU::Register
	{
		Null = 0,
		PushStack,
		PopStack,
		DropStack,
		DropCash,
		ToEnd,

	};
	bool Update() {

		if (Q.size() == 0) { return false; }

		auto& N = Q.front();
		Q.pop_front();

		switch (std::get<0>(N))
		{
		case Ops::Null:
			break;
		case Ops::Load:
			R[std::get<1>(N)] = Memory[std::get<2>(N)];
			break;
		case Ops::Store:
			Memory[std::get<1>(N)] = R[std::get<2>(N)];
			break;
		case Ops::Move:
			R[std::get<1>(N)] = R[std::get<2>(N)];
			break;
		case Ops::Set:
			R[std::get<1>(N)] = std::get<2>(N);
			break;
		case Ops::And:
			R[0] = R[std::get<1>(N)] & R[std::get<2>(N)];
			break;
		case Ops::Or:
			R[0] = R[std::get<1>(N)] | R[std::get<2>(N)];
			break;
		case Ops::Xor:
			R[0] = R[std::get<1>(N)] ^ R[std::get<2>(N)];
			break;
		case Ops::Equal:
			R[0] = ~(R[std::get<1>(N)] ^ R[std::get<2>(N)]);
			break;
		case Ops::Not:
			R[0] = ~R[std::get<1>(N)];
			break;
		case Ops::Nor:
			R[0] = ~(R[std::get<1>(N)] | R[std::get<2>(N)]);
			break;
		case Ops::Nand:
			R[0] = ~(R[std::get<1>(N)] & R[std::get<2>(N)]);
			break;
		case Ops::Add:
			R[0] = R[std::get<1>(N)] + R[std::get<2>(N)];
			break;
		case Ops::Sub:
			R[0] = (R[std::get<1>(N)] - R[std::get<2>(N)]);
			break;
		case Ops::Mul:
			R[0] = (R[std::get<1>(N)] * R[std::get<2>(N)]);
			break;
		case Ops::Div:
			R[0] = (R[std::get<1>(N)] / R[std::get<2>(N)]);
			break;
		case Ops::Mod:
			R[0] = (R[std::get<1>(N)] % R[std::get<2>(N)]);
			break;
		case Ops::Test:
			R[0] = ~(std::get<1>(N) ^ std::get<2>(N));
			break;
		case Ops::LShift:
			R[0] = R[std::get<1>(N)] << std::get<2>(N);
			break;
		case Ops::RShift:
			R[0] = R[std::get<1>(N)] >> std::get<2>(N);
			break;
		case Ops::Hit: {
			Register A{ 1 };
			R[0] = (R[std::get<1>(N)] & (A << std::get<2>(N))) ? 1 : 0;
			break;
		}
		case Ops::RWin:
			R[0] = (R[std::get<1>(N)] < std::get<2>(N)) ? 1 : 0;
			break;
		case Ops::LWin: 
			R[0] = (R[std::get<1>(N)] >  std::get<2>(N)) ? 1 : 0;
			break;
		case Ops::BWin: ;
			R[0] = (R[std::get<1>(N)] ==  std::get<2>(N))? 1 : 0;
			break;
		case Ops::Int:
			IntegralService((IntOps)std::get<1>(N), std::get<2>(N));
			break;
		default:
			return false;
		}
		return true;
	}

	bool IntegralService(IntOps IO, VirtualCPU::Register Re) {

		switch (IO) {
			{
		case TestCPU::IntOps::Null:
			break;
		case TestCPU::IntOps::DropStack:
			Stack.pop_back();
			break;
		case TestCPU::IntOps::PushStack:
			Stack.push_back(R);
			break;
		case TestCPU::IntOps::PopStack:
			R = Stack.back();
			Stack.pop_back();
			break;
		case TestCPU::IntOps::ToEnd:
			VirtualCPU::ToEnd = (Re != 0);
			break;
		case TestCPU::IntOps::DropCash:
		{
			decltype(Q)::value_type A = Q.front();
			Q.clear();
			Q.push_back(A);
		}
			break;
		default:
			return false;
			break;
			}

		}
		return true;
	}
};

int main() {
	TestCPU::spVirtualCPU TC = std::make_shared<TestCPU>();

	TC->Initialize(16, 256);

	(*TC)[0] = 0x1001;
	(*TC)[1] = 0x1011;
	(*TC)[2] = 4;
	(*TC)[3] = 2;

	TC->Push({ Ops::Load,1,0 });
	TC->Push({ Ops::Load,2,1 });

	TC->Push({ Ops::And, 1, 2 });
	TC->Push({ Ops::Or,1,2 });
	TC->Push({ Ops::Xor ,1 ,2 });
	TC->Push({ Ops::Not, 1, 0 });

	TC->Push({ Ops::Nand, 1, 2 });
	TC->Push({ Ops::Nor,1,2 });
	TC->Push({ Ops::Equal ,1 ,2 });

	TC->Push({ Ops::Load,1,2 });
	TC->Push({ Ops::Load,2,3 });

	TC->Push({ Ops::Add, 1, 2 });
	TC->Push({ Ops::Sub,1,2 });
	TC->Push({ Ops::Mul ,1 ,2 });
	TC->Push({ Ops::Div ,1,2 });
	TC->Push({ Ops::Mod ,1 ,2 });

	TC->Push({ Ops::Set ,3 ,0xbeef });
	TC->Push({ Ops::Hit ,3 ,2 });

	TC->Push({ Ops::Int ,(TestCPU::Register)TestCPU::IntOps::ToEnd ,1 });


	while (TC->IsEnd()==false) {
		TC->Update();
		std::cout <<std::hex<< TC->Registers(0) << std::endl;
	}
	return 0;

}