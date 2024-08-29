import os, sys, json

import google.generativeai as genai

from langchain.text_splitter import RecursiveCharacterTextSplitter
from langchain.vectorstores import Chroma
from langchain_community.document_loaders import TextLoader, DirectoryLoader
from langchain.embeddings import HuggingFaceEmbeddings

model_name = "jhgan/ko-sbert-nli"
model_kwargs = {'device': 'cpu'}
encode_kwargs = {'normalize_embeddings': True}
hf = HuggingFaceEmbeddings(
    model_name=model_name,
    model_kwargs=model_kwargs,
    encode_kwargs=encode_kwargs
)

# 참고: 컨텍스트 캐싱은 고정 버전 (예: gemini-1.5-pro-001)의 안정적인 모델에서만 사용할 수 있습니다. 버전 접미사를 포함해야 합니다 (예: gemini-1.5-pro-001의 -001).

GOOGLE_API_KEY= 'AIzaSyC6D0cvniIKkJWLbexK3TPZsTl16zHEMLA'
genai.configure(api_key=GOOGLE_API_KEY)

# result = []
# for m in genai.list_models():
#     if 'generateContent' in m.supported_generation_methods:
#         result.append(m.name)

model = genai.GenerativeModel('gemini-1.5-flash')

import webuiapi
from enum import Enum
# create API client
api = webuiapi.WebUIApi()

def getModelList():
    return api.util_get_model_names()

def setModel(args):
    api.util_set_model(args['model'])

# layerdiffusion_enabled
# layerdiffusion_method
# layerdiffusion_weight
# layerdiffusion_ending_step
# layerdiffusion_fg_image
# layerdiffusion_bg_image
# layerdiffusion_blend_image
# layerdiffusion_resize_mode
# layerdiffusion_fg_additional_prompt
# layerdiffusion_bg_additional_prompt
# layerdiffusion_blend_additional_prompt

def sdCall(args):
    print(args)
    scr = {}
    if args['background'] == False and args['version'] == '1.5':
        scr = {
                        "LayerDiffuse": {
                            "args": [
                                True,
                                "(SD1.5) Only Generate Transparent Image (Attention Injection)",
                                1,
                                1,
                                None,
                                None,
                                None,
                                "Crop and Resize",
                                "",
                                "",
                                ""
                            ]
                        }
                    }
    elif args['background'] == False and args['version'] == 'XL':
        scr = {
                "LayerDiffuse": {
                    "args": [
                        True,
                        "(SDXL) Only Generate Transparent Image (Attention Injection)",
                        1,
                        1,
                        None,
                        None,
                        None,
                        "Crop and Resize",
                        "",
                        "",
                        ""
                    ]
                }
            }

    result1 = api.txt2img(prompt=args['prompt'],
                    negative_prompt=args['negative_prompt'],
                    seed=args['seed'],
                    cfg_scale=args['cfg'],
                    sampler_index=args['sampler'],
                    steps=args['steps'],
                    width=args['width'],
                    height=args['height'],
                    enable_hr=args['upscale'],
                    hr_scale=2,
                    hr_upscaler=webuiapi.HiResUpscaler.ESRGAN_4x,
                    hr_second_pass_steps=20,
                    hr_resize_x=args['width'],
                    hr_resize_y=args['height'],
                    denoising_strength=0.4,
                    alwayson_scripts=scr)

    result1.images[-1].save("sd.png")

def apiCall(body):
    response = model.generate_content(body)
    return response.text

def apiRAG(body):
    text_loader_kwargs={'autodetect_encoding': True}
    loader = DirectoryLoader('ragdb', glob="**/*.txt", loader_cls=TextLoader, loader_kwargs=text_loader_kwargs)
    data = loader.load_and_split()

    text_splitter = RecursiveCharacterTextSplitter(chunk_size=500, chunk_overlap=50)
    texts = text_splitter.split_documents(data)

    docsearch = Chroma.from_documents(texts, hf)

    retriever = docsearch.as_retriever(
                                    search_type="mmr",
                                    search_kwargs={'k':3, 'fetch_k': 10})
    result = retriever.get_relevant_documents(body)
    response = [x.dict()['page_content'] for x in result]
    return str(response)

from flask import Flask, request, make_response, send_file
app = Flask(__name__)

@app.route('/')
def hello_world():
    return 'true'

@app.route('/InitAPI', methods=['GET'])
def InitAPI():
    file_list = os.listdir("./ragdb")

    for file in file_list:
        os.remove("./ragdb/" + file)
    return 'true'

@app.route('/GetSDModels', methods=['GET'])
def ModelList():
    return getModelList()

@app.route('/GetSamplers', methods=['GET'])
def SamplerList():
    return api.get_samplers()

@app.route('/GetProgress', methods=['GET'])
def CurProgress():
    return api.get_progress()

@app.route('/GetLoRAs', methods=['GET'])
def LoRAList():
    return os.listdir(api.get_cmd_flags()['data_dir'] + "/models/Lora")

@app.route('/SetSDModel', methods=['POST'])
def SetSDModel():
    params = json.loads(request.get_data().decode('utf-8'))
    args = params['args']
    setModel(args)
    return 'true'

@app.route('/CallSD', methods=['POST'])
def CallSD():
    params = json.loads(request.get_data().decode('utf-8'))
    args = params['args']
    sdCall(args)
    return send_file("sd.png", mimetype="image/png", as_attachment=True)

@app.route('/CallAPI', methods=['POST'])
def CallAPI():
    return apiCall(request.get_data().decode('utf-8'))

@app.route('/CallRAG', methods=['POST'])
def CallRAG():
    return apiRAG(request.get_data().decode('utf-8'))

@app.route('/AddRAG', methods=['POST'])
def AddRAG():
    file_list = os.listdir("./ragdb")

    result = {}

    if len(request.get_data().decode('utf-8')) > 4:
        for file in file_list:
            os.remove("ragdb/" + file)

        args = json.loads(request.get_data().decode('utf-8'))

        for e in args:
            f = open("ragdb/" + e + ".txt", 'w', encoding="utf8")
            f.write(args[e])
            f.close()
    else:
        for file in file_list:
            f = open("ragdb/" + file, 'r', encoding="utf8")
            result[os.path.splitext(file)[0]] = f.read()
            f.close()

    return result

@app.route('/DeleteRAG/<name>', methods=['GET'])
def DeleteRAG(name):
    if os.path.isfile("ragdb/" + name + ".txt"):
        os.remove("ragdb/" + name + ".txt")
    return 'true'

if __name__ == '__main__':
    app.run('0.0.0.0', port=80, debug=True)