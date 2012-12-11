#include "nrg.h"
#include <iostream>

struct TestEntity : EntityHelper<TestEntity, 1337> {
	TestEntity() : f(this){};
	virtual void getFields(nrg::FieldList& list) {
		list.add(f);
	}
	nrg::Field<int> f;
};

int main(void){
	TestEntity te;
	std::cout << te.getType() << std::endl;

	nrg::Entity* te2 = te.clone();
	std::cout << te2->getType() << std::endl;

	return 0;
}
