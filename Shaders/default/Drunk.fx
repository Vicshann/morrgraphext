//Author: Scanti
//Merges the screen together with the last frame, and then blurs it.

texture lastframe;
texture lastpass;

PixelShader PS = asm
{
	ps.1.4
	def c0,0.00125,0,0,1 //Used to mod r0,r2 (Use these value for a 800,600 resolution) = 1/800
	def c1,0,0.00166667,0,1 //Used to mod r1,r3 = 1/600
	def c2,1.5,1.5,1.5,0.5 //Used to set the alpha channel
	texcrd r0.rgb,t0  //Get texture coordinates
	mov r0.a,c2  //And put a value in r0.a so the validater doesn't complain about uninitalized registers
	sub r3,r0,c1  //Copy the texture coords into the first four registers and shift them slightly
	sub r2,r0,c0
	add r1,r0,c1
	add r0,r0,c0
	phase    //Second phase
	texld r0,r0   //Sample the backbuffer at the four different points
	texld r1,r1
	texld r2,r2
	texld r3,r3
	add r0,r0,r1  //Add 'em all up and divide by 4
	add r0,r0,r2
	add_d4 r0,r0,r3
};

Technique T0
{
	Pass P0
	{
		Texture[1]=<lastframe>;
		PixelShader=asm
		{
			ps.1.1
			tex t0
			tex t1
			add_d2 r0,t0,t1
		};
	}
	Pass P1
	{
		Texture[0] = <lastpass>; //We want to sample the results of the last pass so that it gets more blured with each pass
		Texture[1] = <lastpass>;
		Texture[2] = <lastpass>;
		Texture[3] = <lastpass>;
		PixelShader = <PS>;  //Use the same predifined pixelshader for all further passes
	}
	Pass P2 { PixelShader = <PS>; }
	Pass P3 { PixelShader = <PS>; }
	Pass P4 { PixelShader = <PS>; }
	Pass P5 { PixelShader = <PS>; }
	Pass P6 { PixelShader = <PS>; }
	Pass P7 { PixelShader = <PS>; }
	Pass P8 { PixelShader = <PS>; }
	Pass P9 { PixelShader = <PS>; }
}
