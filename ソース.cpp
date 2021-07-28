#include <iostream>
#include <cstdint>
#include <vector>
#include <deque>
#include <tuple>
#include <memory>
#include <algorithm>
#include <map>

#include <iomanip> 

//many test is not complete.confuse...
//たのしい～！！が管理限界がちかいぞ・・・。

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
	HALT,
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
	Inc,
	Dec,
	Address, //Addressing like a pointer.
	HighLimit,//Clamp HighLimit.
	LowLimit,//Clamp LowLimit.
	Swap,// swap to register and register.
	//Clamp,//[Min,Max]
	Hit,//one of Bit Test
	RGreater,//R is greater.
	LGreater,//L is greater.
	BGreater,//Both Win.
	LShift,//bit shift left.
	RShift,//bit shift right.
	RSet,//[RegisterPos,Value]//initialize Register.
	MSet,//[MemoryPos,Value]//initialize memory.
	Label,//Label for Jamp.
	JNZ,//jamp if non zero.[Rsgister,Label].
	JIZ,//jamp if zero.[Register,Label].
	Test,//BitTest by tow value. answer is yet unknown.
	Int,//Special Service.
	Load,//from Memory.
	Store,//To Memory
	Move,//register to register.
	EnumEnd,//Terminater of this Enum.
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
		ReWindPC,
		SaveMemoryBlock,//lolololol;
		LoadMemoryBlock,//lolololol;
		ToEnd,

	};
	bool Initialize(std::size_t RS,std::size_t MS) {//different to call this.
		VirtualCPU* VC = this;

		VC->Initialize(RS, MS);
		PC = 0;
		LS.clear();
		B.clear();
	}
	bool Update() {

		if (Q.size() == 0) { return false; }

		if (PC >= Q.size()) { return false; }

		auto& N = Q[PC++];
		//Q.pop_front();

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
		case Ops::RSet:
			R[std::get<1>(N)] = std::get<2>(N);
			break;
		case Ops::MSet:
			Memory[std::get<1>(N)] = std::get<2>(N);
			break;
		case Ops::Address:
			R[0] = Memory[R[std::get<1>(N)]];
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
		case Ops::Swap: {
			Register A = R[std::get<1>(N)];
			R[std::get<1>(N)] = R[std::get<2>(N)];
			R[std::get<2>(N)] = A;
			break;
		}
		case Ops::Inc:
			 R[std::get<1>(N)]++;
			break;
		case Ops::Dec:
			 R[std::get<1>(N)]--;
			break;
		case Ops::HighLimit:
			R[0] = std::min(R[std::get<1>(N)] ,  R[std::get<2>(N)]);
			break;
		case Ops::LowLimit:
			R[0] = std::max(R[std::get<1>(N)] ,  R[std::get<2>(N)]);
			break;
		case Ops::Test:
			R[0] = ~(R[std::get<1>(N)] ^ R[std::get<2>(N)]);
			break;
		case Ops::LShift:
			R[0] = R[std::get<1>(N)] << R[std::get<2>(N)];
			break;
		case Ops::RShift:
			R[0] = R[std::get<1>(N)] >> R[std::get<2>(N)];
			break;
		case Ops::Hit: {
			Register A{ 1 };
			R[0] = (R[std::get<1>(N)] & (A << R[std::get<2>(N)])) ? 1 : 0;
			break;
		}
		case Ops::RGreater:
			R[0] = (R[std::get<1>(N)] < R[std::get<2>(N)]) ? 1 : 0;
			break;
		case Ops::LGreater: 
			R[0] = (R[std::get<1>(N)] >  R[std::get<2>(N)]) ? 1 : 0;
			break;
		case Ops::BGreater:
			R[0] = (R[std::get<1>(N)] ==  R[std::get<2>(N)])? 1 : 0;
			break;

		case Ops::Label: {
			std::tuple<std::size_t, Register> A{ PC, std::get<1>(N) };
			auto it=std::find(LS.begin(), LS.end(),A);
			
			if (it == LS.end()) {
				LS.push_back({ PC, std::get<1>(N) });
				//std::sort(LS.begin(), LS.end());
			}
			break;
		}
		case Ops::JIZ:
			if (!R[std::get<1>(N)]) {
				for (auto& o : LS) {
					if (std::get<1>(o) == std::get<2>(N)) {
						PC = std::get<0>(o);
						break;
					}
				}
			}
			break;
		case Ops::JNZ:
			if (R[std::get<1>(N)]) {
				for (auto& o : LS) {
					if (std::get<1>(o) == std::get<2>(N)) {
						PC = std::get<0>(o);
						break;
					}
				}
			}
			break;
		case Ops::Int:
			IntegralService(std::get<1>(N), R[std::get<2>(N)]);
			break;
		default:
			Ops::EnumEnd;
			return false;
		/** /
		case Ops::Clamp: {
			if (R[std::get<1>(N)] > R[std::get<2>(N)]) {//i want to del this if.
				Register A = std::get<1>(N);
				std::get<2>(N) = std::get<1>(N);
				std::get<1>(N) = A;
			}
			R[0] = std::min(R[std::get<2>(N)], std::max(R[std::get<1>(N)], R[0]));
			break;
		}	
		/**/
		}
		return true;
	}

	bool IntegralService(VirtualCPU::Register SN, VirtualCPU::Register Re) {

		IntOps IO = (IntOps)SN;

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
		case TestCPU::IntOps::SaveMemoryBlock:
			B[Re] = Memory;
			break;
		case TestCPU::IntOps::LoadMemoryBlock:{
			std::size_t L = Memory.size();
			Memory=B[Re];
			Memory.resize(L);
			break;
		}
		case TestCPU::IntOps::ReWindPC:
			PC = Re;
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
	std::size_t ProgramableCounter() {
		return PC;
	}
protected:
	std::map<Register,std::vector<Register>> B;
	std::vector < std::tuple<std::size_t, Register>> LS;//label stack.
	std::size_t PC{ 0 };
};

int main() {
	TestCPU::spVirtualCPU TC = std::make_shared<TestCPU>();

	TC->Initialize(16, 256);

	TC->Push({ Ops::MSet, 0, 0x1001 });
	TC->Push({ Ops::MSet, 1, 0x1010 });
	TC->Push({ Ops::MSet, 2, 4 });
	TC->Push({ Ops::MSet, 3, 2 });
	//(*TC)[0] = 0x1001;
	//(*TC)[1] = 0x1011;
	//(*TC)[2] = 4;
	//(*TC)[3] = 2;

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

	TC->Push({ Ops::RSet ,3 ,0xbeef });
	TC->Push({ Ops::Hit ,3 ,2 });

	TC->Push({ Ops::Int ,(TestCPU::Register)TestCPU::IntOps::ToEnd ,1 });


	while (TC->IsEnd()==false) {
		TC->Update();
		std::cout <<std::hex<< TC->Registers(0) << std::endl;
	}
	return 0;

}