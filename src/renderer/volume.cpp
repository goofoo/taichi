#include "volume.h"

TC_NAMESPACE_BEGIN

TC_INTERFACE_DEF(VolumeMaterial, "volume_material")

class VacuumVolumeMaterial : public VolumeMaterial {
	virtual void initialize(const Config &config) override {
		this->volumetric_scattering = 0.0f;
		this->volumetric_absorption = 0.0f;
	}
	virtual real get_attenuation(real dist) const override {
		return 1.0f;
	}

	virtual real unbiased_sample_attenuation(const Vector3 &start, const Vector3 &end, StateSequence &rand) const override {
		return 1.0f;
	}

	virtual VolumeEvent sample_event(StateSequence &rand) const override {
		error("invalid");
		return VolumeEvent::absorption;
	}
};

VolumeStack::VolumeStack() {
	static std::shared_ptr<VolumeMaterial> vacuum = create_instance<VolumeMaterial>("vacuum");
	stack.push_back(vacuum.get());
}

TC_IMPLEMENTATION(VolumeMaterial, VolumeMaterial, "homogeneous");
TC_IMPLEMENTATION(VolumeMaterial, VacuumVolumeMaterial, "vacuum");

TC_NAMESPACE_END
