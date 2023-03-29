#version 450

layout(location = 0) in vec3 in_color;
layout(location = 1) in vec2 uv;
layout(location = 2) in float t;
layout(location = 3) in float time;
//   _Sign = clamp(_Sign,-1,1);
//layout(location = 4) in float _Sign;

layout(location = 0) out vec4 outColor;

layout(set = 1, binding = 0) uniform sampler2D u_noise;
layout(set = 1, binding = 1) uniform sampler2D u_imageFlame;
layout(set = 1, binding = 2) uniform sampler2D u_imageFlameColor;



void main() {
	outColor = vec4(in_color, 1.0);
	outColor = vec4(1,1,1, 1.0);
	outColor = texture(u_noise,uv);
	//outColor = texture(u_imageFlameColor,uv);
	//outColor = vec4(uv,0,1);
	float color = sin(time);
	outColor = vec4(t ,t ,t ,1);

	
               
               
                
                vec2 flameUv = uv;
              
                flameUv = flameUv *2 -1;
                vec2 center = vec2(0.0,0.0);
                   
          
                 

                float scale = 4;
                vec2 topCenter = vec2(0.0,-0.35) * scale;
                flameUv=flameUv* scale;
                vec2 tileCoords = fract(flameUv); 
                vec2 tileIndex =  floor(flameUv);
               
                vec2 diff = vec2(tileIndex+tileCoords)-topCenter;
                float value = length(diff)*1;
               
                value /= scale;
               
                float angle = mix(0,25,value);
               
             


                float rotation = 0.4*angle *  3.14/180;
                float sine = sin(rotation);
                float cosine = cos(rotation);
                vec2 pivot = center;

                
            // outColor = vec4(t ,t ,t ,1);
                 
                flameUv -= pivot;
                flameUv.x = flameUv.x * cosine - flameUv.y * sine;
                flameUv.y = flameUv.x * sine + flameUv.y * cosine;
                flameUv += pivot;
               
                flameUv/=scale;
                flameUv = flameUv /2 +0.5;
              

                float amplitude = 0.2;
                float movement =0;
                float frequency = 1.;
            
                float tTemp = 0.01*(-time*130.0);
                movement += sin(flameUv.y*frequency*2.1 + tTemp)*4.5;
                movement += sin(flameUv.y*frequency*1.72 + tTemp*1.121)*4.0;
                movement += sin(flameUv.y*frequency*2.221 +tTemp*0.437)*5.0;
                movement += sin(flameUv.y*frequency*3.1122+ tTemp*4.269)*2.5;
                movement *= amplitude*0.06;
                
				flameUv.x += movement*0.4*value ;

                outColor =vec4(flameUv,0,1);
              
                
                vec4 tex = texture(u_imageFlame,flameUv);
				
                vec2 x_range = vec2(-1,1);
                flameUv.x = mix(x_range.x, x_range.y, flameUv.x);
                flameUv.y -= cos(4*flameUv.x) ;
                flameUv.y -= time;
                
            
                
				vec4 noise = texture(u_noise,flameUv);
				//outColor = vec4(noise);
             
                
                vec4  noiseInv =  vec4( vec3(1-noise.rgb) , noise.a);
                vec4 noiseAdd = vec4(  (noise.rgb) ,noise.a);
              
                
                vec4 res = mix(vec4(0),noiseAdd,tex);
               

               

                res.rgb*=t;
 				
				
                float x= res.r;
                float thresh = 0.09;
                res.rgb = step(thresh ,res.rgb);
                res.a = res.x;
				
                vec4 blackWhiteGrad = mix(vec4(1,1,1,res.a),vec4(0,0,0,res.a),res);
				vec4 colorTex = texture(u_imageFlameColor,vec2(x,t));
				outColor= res* colorTex;
              

                
              


}