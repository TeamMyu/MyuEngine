for d in "$( dirname -- "$0"; )"/* ; do
if [ "$( basename -- "$d" )" != "compile.sh" ]; then
echo "$( basename -- "$d" )" was compilled
glslc $d -o $1bin\\Editor\\Debug\\shaders\\$( basename -- "$d" ).spv
fi
done
sleep 1