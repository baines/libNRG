#include "nrg.h"

struct TestEntity : public nrg::Entity {
	TestEntity() : f(this){};
	virtual TestEntity* clone() const { return new TestEntity(); }
	virtual uint16_t getType() const { return 1; }
	virtual void getFields(std::vector<nrg::FieldBase*>& vec) {
		vec.push_back(&f);
	}
	nrg::Field<int> f;
};

int main(void){
	TestEntity te;

	return 0;
}
