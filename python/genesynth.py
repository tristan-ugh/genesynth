import ctypes
import os
import platform
import numpy as np

class GeneSynth:
    def __init__(self, lib_path=None, sample_rate=22050.0):
        """
        Initializes the headless GeneSynth engine.
        """
        if lib_path is None:
            # Guess the default build path
            system = platform.system()
            if system == "Windows":
                lib_name = "genesynth_python_api.dll"
                search_paths = [
                    os.path.join("..", "build-win", "Release", lib_name),
                    os.path.join("..", "build-win", "Debug", lib_name),
                    os.path.join("build-win", "Release", lib_name),
                ]
            else:
                lib_name = "libgenesynth_python_api.so"
                search_paths = [
                    os.path.join("..", "build", lib_name),
                    os.path.join("build", lib_name),
                ]
            
            for p in search_paths:
                if os.path.exists(p):
                    lib_path = p
                    break
            
            if lib_path is None:
                raise FileNotFoundError(f"Could not find {lib_name} in standard build directories.")

        # Load the library
        self.lib = ctypes.CDLL(os.path.abspath(lib_path))
        
        # Setup signatures
        self.lib.GeneSynth_Create.argtypes = [ctypes.c_double]
        self.lib.GeneSynth_Create.restype = ctypes.c_void_p
        
        self.lib.GeneSynth_Destroy.argtypes = [ctypes.c_void_p]
        self.lib.GeneSynth_Destroy.restype = None
        
        self.lib.GeneSynth_GetNumParameters.argtypes = [ctypes.c_void_p]
        self.lib.GeneSynth_GetNumParameters.restype = ctypes.c_int
        
        self.lib.GeneSynth_GetParameterId.argtypes = [ctypes.c_void_p, ctypes.c_int]
        self.lib.GeneSynth_GetParameterId.restype = ctypes.c_char_p
        
        self.lib.GeneSynth_RenderFeatures.argtypes = [ctypes.c_void_p, ctypes.POINTER(ctypes.c_float), ctypes.POINTER(ctypes.c_float)]
        self.lib.GeneSynth_RenderFeatures.restype = None
        
        # Create instance
        self.instance = self.lib.GeneSynth_Create(float(sample_rate))
        if not self.instance:
            raise RuntimeError("Failed to create GeneSynth instance")
            
        # Get parameter info
        self.num_params = self.lib.GeneSynth_GetNumParameters(self.instance)
        self.param_ids = []
        for i in range(self.num_params):
            pid = self.lib.GeneSynth_GetParameterId(self.instance, i).decode('utf-8')
            self.param_ids.append(pid)
            
    def __del__(self):
        if hasattr(self, 'instance') and self.instance and hasattr(self, 'lib'):
            self.lib.GeneSynth_Destroy(self.instance)
            
    def get_parameter_ids(self):
        """Returns the list of parameter names in the exact order the C++ engine expects."""
        return self.param_ids
        
    def render(self, params_dict=None, params_array=None):
        """
        Renders 1 second of audio and returns the Log-Mel Spectrogram.
        You can pass either:
        - params_dict: dict of {param_id: float_value}
        - params_array: list or numpy array of length `num_params`
        
        Returns:
            numpy.ndarray of shape (86, 128) containing the spectrogram.
        """
        if params_array is None:
            # Default to 0.5 for everything if not specified
            c_params = (ctypes.c_float * self.num_params)(*[0.5]*self.num_params)
            
            if params_dict:
                for i, pid in enumerate(self.param_ids):
                    if pid in params_dict:
                        c_params[i] = params_dict[pid]
        else:
            if len(params_array) != self.num_params:
                raise ValueError(f"Expected {self.num_params} parameters, got {len(params_array)}")
            c_params = (ctypes.c_float * self.num_params)(*params_array)
            
        # Allocate output buffer
        out_frames = 86
        out_mels = 128
        out_size = out_frames * out_mels
        c_out = (ctypes.c_float * out_size)()
        
        self.lib.GeneSynth_RenderFeatures(self.instance, c_params, c_out)
        
        # Convert to numpy
        spec = np.ctypeslib.as_array(c_out).copy()
        return spec.reshape((out_frames, out_mels))
