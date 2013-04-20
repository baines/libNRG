#include "nrg.h"
#include <iostream>

struct TestEntity : nrg::EntityHelper<TestEntity, 1337> {
	TestEntity() : f(this){};
	nrg::Field<int> f;
};

int main(void){
	// Test that an Entity can be created via EntityHelper
	TestEntity te;
	// Its type should be 1337
	printf("%u\n", te.getType());

	// Test that Entity::clone() works as expected.
	// The type should also be 1337
	nrg::Entity* te2 = te.clone();
	printf("%u\n", te2->getType());

	return 0;
}
