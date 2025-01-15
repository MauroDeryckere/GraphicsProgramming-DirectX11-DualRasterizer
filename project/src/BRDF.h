#pragma once

#include "Math.h"
#include "Vector3.h"
#include "ColorRGB.h"

namespace dae
{
	namespace BRDF
	{
		/**
		 * \param kd Diffuse Reflection Coefficient
		 * \param cd Diffuse Color
		 * \return Lambert Diffuse Color
		 */
		static ColorRGB Lambert(float kd, const ColorRGB& cd)
		{
			return { (kd * cd) / PI };
		}

		static ColorRGB Lambert(const ColorRGB& kd, const ColorRGB& cd)
		{
			return { (kd * cd) / PI };
		}

		/**
		 * \param ks Specular Reflection Coefficient
		 * \param exp Phong Exponent
		 * \param l Incoming (incident) Light Direction
		 * \param v View Direction
		 * \param n Normal of the Surface
		 * \return Phong Specular Color
		 */
		static ColorRGB Phong(float ks, float exp, const Vector3& l, const Vector3& v, const Vector3& n)
		{
			Vector3 const reflect{ Vector3::Reflect(l, n) };
			float const alpha{ Vector3::Dot(-reflect, v) };
			if (alpha > 0)
			{
				auto const phong = ks * (powf(alpha, exp));
				return { phong, phong, phong };
			}
			return colors::Black;
		}
	}
}