#pragma once
#include "Technique.h"

class CIBLTechnique : public CTechnique
{
public:
	CIBLTechnique(void);
	virtual ~CIBLTechnique(void);

	virtual bool initTechniqueV() override;
};