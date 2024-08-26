git clone https://github.com/lllyasviel/stable-diffusion-webui-forge

OS_TYPE=$(uname -s)

if [ "$OS_TYPE" = "Darwin" ]; then
    echo "This is macOS."
    /bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"
    brew install python@3.10
    
    python3.10 -m venv venv
    source ./venv/bin/activate
    ./venv/bin/pip3 install -r requirements.txt
    
    sh ./stable-diffusion-webui-forge/webui.sh --always-cpu --api --allow-code --xformers --skip-torch-cuda-test --no-half-vae --precision full --no-half --use-cpu &

elif [ "$OS_TYPE" = "Linux" ]; then
    apt install python3.10-venv -y
    python3.10 -m venv venv
    source ./venv/bin/activate
    ./venv/bin/pip3 install -r requirements.txt

    sh ./stable-diffusion-webui-forge/webui.sh --api --allow-code --xformers --skip-torch-cuda-test --no-half-vae --precision full --no-half &
else
    echo "Unknown operating system: $OS_TYPE"
fi

python3.10 server.py &

