import pytest
import os
import sys
import subprocess
import inkcpp_py as ink
import warnings

@pytest.fixture(scope='session', autouse=True)
def inklecate_cmd():
    res = os.getenv("INKLECATE")
    if res is None or res == "":
        res = "inklecate"
    try:
        output = subprocess.run([res], capture_output=True)
        is_inklecate = output.stdout.decode("utf-8").split("\n")[0].find("inklecate") > -1
    except FileNotFoundError as err:
        pytest.fail(f"Unable to find '{res}', needed to compile .ink to .ink.json\n\tErr ({err.errno}): {err.strerror}\n\ttry setting the correct inklecate executable via the INKLECATE enviroment variable.")
        return None
    except OSError as err:
        pytest.fail(f"Unable to execute '{res}', needed to compile .ink to .ink.json\n\tMsg ({err.errno}): {err.strerror}\n\ttry setting the correct inklecate executable via the INKLECATE enviroment variable.")
        return None
    if not is_inklecate:
        warning.warn(RuntimeWarning(f"Executing '{res}' behaved unexpeted, may results in errors!"))
    return res


def extract_paths(tmpdir):
    def res(ink_source):
        name = os.path.splitext(os.path.basename(ink_source))[0]
        return (
        name,
            list(map(lambda x: tmpdir + ("/" + name + x), [".bin", ".tmp"])) + ["./inkcpp_test/ink/" + ink_source]
        )
    return res

@pytest.fixture(scope='session', autouse=True)
def story_path(tmpdir_factory):
    tmpdir = tmpdir_factory.getbasetemp()
    # tmpdir = os.fsencode('/tmp/pytest')
    return {name: files 
        for (name, files) in map(extract_paths(tmpdir), 
            filter(
                lambda file: os.path.splitext(file)[1] == ".ink", 
                os.listdir("./inkcpp_test/ink/")))}
            
@pytest.fixture(scope='session', autouse=True)
def assets(story_path, inklecate_cmd):
    res = {}
    print()
    try:
        from tqdm import tqdm
    except ImportError:
        print("Compiling ink scripts takes a while, `pip install tqdm` for a nice progress bar")
        tqdm = lambda x: x
    for (name, files) in tqdm(story_path.items()):
        if not os.path.exists(files[0]):
            if not os.path.exists(files[1]):
                subprocess.run([inklecate_cmd, "-o", files[1], files[2]], check=True)
            ink.compile_json(str(files[1]), str(files[0]))
        res[name] = ink.Story.from_file(str(files[0]))
    return res

@pytest.fixture(scope='session', autouse=True)
def generate():
    def g(asset):
        store = asset.new_globals()
        return [
            asset,
            store,
            asset.new_runner(store),
        ]
    return g
