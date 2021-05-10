
vec4 shade ( vec3 p, vec3 n, vec3 lp, vec3 la, vec3 ld, vec3 ls, vec3 ka, vec3 kd, vec3 ks, vec3 emi, float sh, float alpha )
{
	// CamDev: revise use of lp-p or p:
	//vec3 l = normalize ( lp-p );      // light direction from p
	//vec3 v = normalize ( vec3(0,0,1)-p ); // viewer vector from p
	vec3 l = normalize ( lp );          // light direction from p
	vec3 v = normalize ( vec3(0,0,1) ); // viewer vector from p
	vec3 r = reflect ( -l, n );         // the reflected light ray
	//vec3 r = normalize(l+v);  // testing the half-vector instead of reflected ray

	// LightDev: set Front or BackAndFront rendering as selectable via uniforms
	float dotln = dot(l,n);
	if (dotln<0.0) dotln=-dotln; // to get black in back faces use: max(dotln,0.0)
	float dotvr = dot(v,r);
	if (dotvr<0.0) dotvr=-dotvr; // or: max(dot(v,r),0.0)
	vec3 amb = la * ka;
	vec3 dif = ld * kd * dotln; // lambertian component
	vec3 spe = dotln * ls * ks * pow ( dotvr, sh );
	return vec4 ( amb + dif + spe + emi, alpha );
}
