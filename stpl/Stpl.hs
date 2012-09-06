import Control.Applicative
import qualified Data.Map as M
import Data.Ratio
import qualified Data.Text as T
import System.Console.GetOpt
import System.Environment
import System.Exit
import System.IO
import Text.JSON
import Text.StringTemplate

main :: IO ()
main = getArgs >>= processArgs >>= uncurry processTemplate

processArgs :: [String] -> IO (StringTemplate String, String)
processArgs args = case getOpt RequireOrder options args of
    (actions, positionalArgs, []) -> do
        template <- getTemplate $ foldl (flip id) NoTemplate actions
        modelJSON <- getModel positionalArgs
        return (template, modelJSON)
    (_, _, errors) -> showUsage errors

getTemplate :: TemplateDef -> IO (StringTemplate String)
getTemplate NoTemplate = showUsage ["No template specified\n"]
getTemplate (Template readTemplate) = newSTMP <$> readTemplate
getTemplate (GroupTemplate group "") = showUsage ["Missing name for group " ++ group]
getTemplate (GroupTemplate "" name) = showUsage ["Missing group for name " ++ name]
getTemplate (GroupTemplate group name) = do
    templateGroup <- directoryGroup group
    case getStringTemplate name templateGroup of
        Just template -> return template
        Nothing -> die $ "Error trying to get template " ++ name ++ " from group " ++ group

getModel :: [String] -> IO String
getModel [] = getContents
getModel [file] = readFile file
getModel _ = showUsage ["Too many arguments\n"]

showUsage :: [String] -> IO a
showUsage errors = do
    progName <- getProgName
    let usage = "Usage: " ++ progName ++ " -t STRING        [DATA_FILE]\n" ++
                "       " ++ progName ++ " -f FILE          [DATA_FILE]\n" ++
                "       " ++ progName ++ " -g DIR -n STRING [DATA_FILE]\n"
    die $ usageInfo (concat errors ++ '\n' : usage) options

die :: String -> IO a
die e = hPutStrLn stderr e >> exitFailure

data TemplateDef = NoTemplate | Template (IO String) | GroupTemplate String String

options :: [OptDescr (TemplateDef -> TemplateDef)]
options = [
    Option ['t'] ["template"]
        (ReqArg (\ t _ -> Template $ return t) "STRING")
        "template string to render",
    Option ['f'] ["file"]
        (ReqArg (\ f _ -> Template $ readFile f) "FILE")
        "template file to render",
    Option ['g'] ["group"]
        (ReqArg (\ g d -> GroupTemplate g (case d of GroupTemplate _ n -> n; _ -> "")) "DIR")
        "directory of template files",
    Option ['n'] ["name"]
        (ReqArg (\ n d -> GroupTemplate (case d of GroupTemplate g _ -> g; _ -> "") n) "STRING")
        "name of the root template (with -g)"
  ]

processTemplate :: StringTemplate String -> String -> IO ()
processTemplate template modelJSON = do
    let modelResult = decode $ strip modelJSON :: Result JSValue
    case modelResult of
        Ok model -> putStrLn . render $ withContext template model
        Error err -> die $ "Error while parsing JSON: " ++ err

strip :: String -> String
strip = T.unpack . T.strip . T.pack

instance ToSElem JSValue where
    toSElem JSNull = toSElem (Nothing :: Maybe String)
    toSElem (JSString s) = toSElem (fromJSString s)
    toSElem (JSBool b) = toSElem b
    toSElem (JSRational _ r)
        | denominator r == 1 = toSElem (numerator r)
        | otherwise          = toSElem (fromRational r :: Double)
    toSElem (JSArray l) = toSElem l
    toSElem (JSObject o) = toSElem . M.fromList $ fromJSObject o
