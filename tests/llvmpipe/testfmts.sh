#!/bin/bash

# export GALLIUM_DRIVER=llvmpipe
# export LD_LIBRARY_PATH=/path/to/src/gallium/targets/libgl-xlib
# export PIGLIT_PLATFORM=glx

declare prog="$PWD/bin/llvmpipe"

declare -a width=(128 256)
declare -a formats=(GL_RED GL_RG GL_RGB GL_RGBA)
declare -a types=(GL_BYTE GL_SHORT GL_HALF_FLOAT GL_FLOAT)
declare -a internalformats=(GL_R8 GL_RG8 GL_RGB8 GL_RGBA8 GL_R16 GL_RG16 GL_RGB16 GL_RGBA16 GL_R16F GL_RG16F GL_RGB16F GL_RGBA16F GL_R32F GL_RG32F GL_RGB32F GL_RGBA32F)
declare count=20

echo 'Standard formats'
for (( k = 0; k < ${#width[@]}; k++ )); do
	echo 'LP_NATIVE_VECTOR_WIDTH=' ${width[$k]}
	export LP_NATIVE_VECTOR_WIDTH=${width[$k]}

	for (( i = 0; i < ${#types[@]}; i++ )); do
		for (( j = 0; j < ${#formats[@]}; j++ )); do
			internalformat=${internalformats[$i * 4 + $j]}
			echo ${types[$i]} ${formats[$j]} $internalformat
			eval $prog '-type' ${types[$i]} '-format' ${formats[$j]} '-internalformat' $internalformat '-auto -count' $count '-clamped'
		done
	done

	TYPE=GL_UNSIGNED_BYTE
	FMT=GL_BGRA
	IFMT=GL_RGBA8
	echo $TYPE $FMT $IFMT
	eval $prog '-type' $TYPE '-format' $FMT '-internalformat' $IFMT '-auto -count' $count '-clamped'
done

declare -a alpha=(GL_ALPHA GL_ALPHA4 GL_ALPHA8 GL_ALPHA12 GL_ALPHA16)
declare -a luminance=(GL_LUMINANCE GL_LUMINANCE4 GL_LUMINANCE8 GL_LUMINANCE12 GL_LUMINANCE16)
declare -a luminance_alpha=(GL_LUMINANCE_ALPHA GL_LUMINANCE4_ALPHA4 GL_LUMINANCE6_ALPHA2 GL_LUMINANCE8_ALPHA8 GL_LUMINANCE12_ALPHA4 GL_LUMINANCE12_ALPHA12 GL_LUMINANCE16_ALPHA16)
declare -a intensity=(GL_INTENSITY GL_INTENSITY4 GL_INTENSITY8 GL_INTENSITY12 GL_INTENSITY16)
declare -a color=(GL_R3_G3_B2 GL_RGB4 GL_RGB5 GL_RGB10 GL_RGB12 GL_RGBA2 GL_RGBA4 GL_RGB5_A1 GL_RGB10_A2 GL_RGBA12)
declare -a stypes=(GL_SLUMINANCE GL_SLUMINANCE8 GL_SLUMINANCE_ALPHA GL_SLUMINANCE8_ALPHA8 GL_SRGB GL_SRGB8 GL_SRGB_ALPHA GL_SRGB8_ALPHA8);
declare -a alt_fmts=()

alt_fmts+=(${color[@]})
alt_fmts+=(${alpha[@]})
alt_fmts+=(${intensity[@]})
alt_fmts+=(${luminance[@]})
alt_fmts+=(${luminance_alpha[@]})
alt_fmts+=(${stypes[@]})

echo 'Alternate formats'
for (( k = 0; k < ${#width[@]}; k++ )); do
	echo "LP_NATIVE_VECTOR_WIDTH=${width[$k]}"
	export LP_NATIVE_VECTOR_WIDTH=${width[$k]}

	for (( j = 0; j < ${#alt_fmts[@]}; j++ )); do
		# type and format doesn't seem to matter for llvmpipe + nvidia
		TYPE=GL_FLOAT
		FMT=GL_RGBA
		IFMT=${alt_fmts[j]}
		echo $TYPE $FMT $IFMT
		eval $prog '-type' $TYPE '-format' $FMT '-internalformat' $IFMT '-auto -count' $count '-clamped'
	done
done
