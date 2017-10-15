#include "PhysicalMaterial.h"


PhysicalMaterial::~PhysicalMaterial() 
{
	if (m_texture != nullptr)
		delete m_texture;
}

Colour PhysicalMaterial::FetchBaseColour(const Scene* scene, Ray ray, PixelHitInfo& hit, int recursionCount)
{
	Colour colour;
	if(m_texture == nullptr)
		return GetColour();
	else
		return m_texture->GetColour(hit.uvs.x, hit.uvs.y) * GetColour();
}

Colour PhysicalMaterial::FetchColour(const Scene* scene, Ray ray, PixelHitInfo& hit, int recursionCount)
{
	Colour baseColour = FetchBaseColour(scene, ray, hit, recursionCount);;

	// Don't do complex checks if only simple rendering
	if (scene->IsSimpleRenderingEnabled())
		return baseColour;


	// Get reflection colour
	if (m_reflectivity != 0.0f)
	{
		vec3 reflRay = reflect(ray.direction, hit.normal);
		Colour reflColour = scene->CalculateRayColour(Ray(hit.location + reflRay * 0.01f, reflRay), recursionCount + 1, scene->GetSkyColour());
		reflColour.a = baseColour.a;
		baseColour = baseColour * (1.0f - m_reflectivity) + reflColour * m_reflectivity;
	}


	const std::vector<Light*>& lights = scene->GetLights();
	Colour totalDiffuse;
	Colour totalSpecular;

	for (Light* light : lights)
	{
		Colour colour;
		float specularFactor;
		light->CalculateLighting(scene, ray, hit, recursionCount, colour, specularFactor);

		totalDiffuse += colour;
		totalSpecular += colour * pow(specularFactor, m_shininess) * m_smoothness;
	}
	

	// Apply specular
	Colour colour = baseColour * (1.0f - baseColour.a) + (baseColour * totalDiffuse * (baseColour.a));
	colour += totalSpecular;
	return ResolveTransparency(colour, scene, ray, hit, recursionCount);
}