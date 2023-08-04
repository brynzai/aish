# AISH
The AI Shell
This shell takes your commands and does what you ask via translating them to intermediate scripting languages and running them. Beware the risk of AI running or deleting something you didn't expect. Use at your own risk. Whatever mode you use you may choose to override the AI temperature via the environment variable: 
`AISH_TEMP` defaults to 0.2. Sessions are not currently persisted to disk and each run is a new thread.

[![YouTube Demo](https://img.youtube.com/vi/8t8u9x9FtdQ/0.jpg)](https://youtu.be/8t8u9x9FtdQ)
# Installing
Latest releases are available for RPM based Linux distros at the BrynzAI COPR: https://copr.fedorainfracloud.org/coprs/boeroboy/brynzai/ 
```
dnf copr enable boeroboy/brynzai
dnf install aish
```
Prerequisites include `libjsoncpp` and `libcurl`.
## Build from Source
Install `jsoncpp-devel` and `libcurl-devel` and simply run `make`.

# Using
Optionally set temperature via `AISH_TEMP` environment variable. The rest depends on the mode being used, which defaults to `shellbard`.
## Modes
Modes are specified with the `-m` arg. Modes fall into two categories: shell or chat. Shell mode tries to translate your input into local commands and runs them immediately. Chat mode is simple chat as you may encounter in the browser. Currently there are for modes supported which are documented below:
1. shellbard
2. shellgpt
3. chatbard
4. chatgpt

Default mode is currently `shellbard`.
## OpenAI GPT Mode
Environment variables are required to use GPT. Currently support is just 3.5-turbo but support for setting the model will be added soon. These should be set to your values:
1. `OPENAI_ORG` your organization ID.
2. `OPENAI_API_KEY` your API key.
There are two options for GPT: `chatgpt` and `shellgpt`.
### chatgpt
This mode is standard chatgpt as used in the browser.
### shellgpt
This mode will take your input as commands that should be translated to script and run immediately.

## Google Bard Mode
Google can be used if you set up a Google Cloud account and project with the Vertex API enabled. Then you will be able to connect using the environment variables:
1. `CLOUDSDK_CORE_PROJECT` set to your project.
2. `GOOGLE_APPLICATION_CREDENTIALS` set to your valid OIDC token.
Note that OIDC tokens may expire frequently and need to be refreshed. There is currently no automation support for this in aish.
### chatbard
Simple chat as with the Bard page. Your identity will also allow management of cloud resources in your GCP project.
### shellbard
Shellbard is the default mode and will translate your input to local shell code and execute it immediately. 

# Scripts
The shell supports scripts. So you can write a script in human language:

```
#!/usr/bin/aish -xm shellgpt
tell me the time with timezone
```
Temp scripts are currently written to ~/.aish/ which is hard-coded to my own user unfortunately. Just trying to get initial commits.
Check the examples directory for more.
```
#!/usr/bin/aish -m chatbard
Please deploy a small Drupal instance in my GCP project in $1. Use Cloud Run if possible. Please do it for me.
```
Running these scripts with parameters becomes simple:
```
./drupal.aish London
Sure, I can do that. Here are the steps I will take:

1. Create a new Cloud Run service in the London region.
2. Deploy the Drupal code to the service.
3. Configure the service to serve traffic on port 80.
4. Create a new DNS record for the service.

Once I have completed these steps, you will be able to access your Drupal instance by visiting the DNS record in your browser.
```
![Example](examples/script.gif)
