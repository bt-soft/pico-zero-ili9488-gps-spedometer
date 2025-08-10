Import("env")

def upload_filesystem(source, target, env):
    env.Execute("pio run --target uploadfs")

# Add filesystem upload as a pre-upload action
env.AddPreAction("upload", upload_filesystem)
