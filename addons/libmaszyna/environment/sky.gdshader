shader_type sky;

/*
I have no idea how to create proper sky shader for MaSzyna panoramas
*/

global uniform sampler2D sky_texture : hint_default_white;
uniform vec2 sky_offset = vec2(0.0, 0.0);
uniform float exposure : hint_range(0.0, 10.0) = 1.0;
uniform vec3 tint_color : source_color = vec3(1.0, 1.0, 1.0);
uniform vec3 sun_color : source_color = vec3( 10.0, 8.0, 1.0 );
uniform vec3 sun_sunset_color : source_color = vec3( 10.0, 0.0, 0.0 );
uniform float sun_size : hint_range( 0.01, 1.0 ) = 0.2;
uniform float sun_blur : hint_range( 0.01, 20.0 ) = 10.0;
uniform vec2 sky_scale = vec2(1.0, 1.0);

void sky() {
	float _moon_amount = 0.0;
	float _horizon_amount = 0.0;

	vec3 color = texture(sky_texture, EYEDIR.xy * sky_scale + sky_offset).rgb;

	color *= tint_color * exposure;


	COLOR = color;


	float _sun_distance = 0.0;
	if( LIGHT0_ENABLED )
	{
		_sun_distance = distance( EYEDIR, LIGHT0_DIRECTION );
		// Bigger sun near the horizon
		float _sun_size = sun_size + cos( LIGHT0_DIRECTION.y * PI ) * sun_size * 0.25;
		// Finding sun disc and edge blur
		float _sun_amount = clamp(( 1.0 - _sun_distance / _sun_size ) / sun_blur, 0.0, 1.0 );
		if( _sun_amount > 0.0 )
		{
			// Changing color of the sun during sunset
			float _sunset_amount = 1.0;
			if( LIGHT0_DIRECTION.y > 0.0 )
				_sunset_amount = clamp( cos( LIGHT0_DIRECTION.y * PI ), 0.0, 1.0 );
			vec3 _sun_color = mix( sun_color, sun_sunset_color, _sunset_amount );
			// Hiding the sun behind the moon
			_sun_amount = clamp( _sun_amount * ( 1.0 - _moon_amount ), 0.0, 1.0 );
			// Hiding the sun behind the horizon
			_sun_amount *= 1.0 - _horizon_amount;
			// Leveling the "glow" in color
			if( _sun_color.r > 1.0 || _sun_color.g > 1.0 || _sun_color.b > 1.0 )
				_sun_color *= _sun_amount;
			COLOR = mix( COLOR, _sun_color, _sun_amount );
		}
	}

}
