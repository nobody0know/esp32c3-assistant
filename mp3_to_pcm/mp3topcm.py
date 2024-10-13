import ffmpeg

def convert_with_soxr(input_file, output_file):
    """
    Convert an MP3 file to PCM format using the soxr resampler with ffmpeg-python.
    
    Args:
    input_file (str): Path to the input MP3 file.
    output_file (str): Path to the output PCM file.
    """
    (
        ffmpeg
        .input(input_file)
        .output(output_file, af='aresample=resampler=soxr', ar=16000, ac=2, format='s16le')
        .run(overwrite_output=True)
    )
    print(f"Converted {input_file} to {output_file} using soxr resampler.")

# Example usage:
convert_with_soxr('a.mp3', 'a.pcm')
