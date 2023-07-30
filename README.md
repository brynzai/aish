# AISH
The AI Shell
This shell takes your commands and does what you ask via translating them to intermediate scripting languages and running them. Beware the risk of AI running or deleting something you didn't expect. Use at your own risk. Whatever mode you use you may choose to override the AI temperature via the environment variable: 
`AISHTEMP` defaults to 0.2. Sessions are not currently persisted to disk and each run is a new thread.

# Modes
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
#!/usr/bin/aish -m shellgpt
tell me the time with timezone
```
Temp scripts are currently written to ~/.aish/ which is hard-coded to my own user unfortunately. Just trying to get initial commits.
More to come soon..
