#ifndef FITNESSAPP_H
#define FITNESSAPP_H

#include "samples/BodyBasics.h"

namespace kf {

	class FitnessApp
	{
	public:
		FitnessApp();
		~FitnessApp();

		int Run();

	private:
		CBodyBasics _bodyBasics;

	};


}


#endif // !FITNESSAPP_H

