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
main = getArgs >>= getConfig >>= validateConfig >>= processTemplate

getConfig :: [String] -> IO Config
getConfig args = case getOpt RequireOrder options args of
    (actions, positionalArgs, []) -> case positionalArgs of
        [] -> return optionsConfig
        [dataFile] -> return optionsConfig {readData = readFile dataFile}
        _ -> usage ["Too many positional arguments\n"]
        where optionsConfig = foldl (flip id) defaultConfig actions
    (_, _, errors) -> usage errors

validateConfig :: Config -> IO Config
validateConfig config = case templateName config of
    "" -> usage ["Missing template option\n"]
    _ -> return config

usage :: [String] -> IO a
usage errors = do
    progName <- getProgName
    mapM_ (hPutStr stderr) errors
    hPutStrLn stderr $ usageInfo ("\nUsage: " ++ progName ++ " -t TEMPLATE [OPTIONS...] [DATA_FILE]") options
    exitFailure

data Config = Config {
    groupPath :: Maybe FilePath,
    templateName :: String,
    readData :: IO String
  }

defaultConfig :: Config
defaultConfig = Config {
    groupPath = Nothing,
    templateName = "",
    readData = getContents
  }

options :: [OptDescr (Config -> Config)]
options = [
    Option ['g'] ["group"]
        (ReqArg (\arg opts -> opts {groupPath = Just arg}) "DIR")
        "Template group directory",

    Option ['t'] ["template"]
        (ReqArg (\arg opts -> opts {templateName = arg}) "TEMPLATE")
        "Template name if using groups, otherwise template path (stdin if not given)"
  ]

processTemplate :: Config -> IO ()
processTemplate config = do
    modelJSON <- readData config
    template <- getTemplate config
    let modelResult = decode $ strip modelJSON :: Result JSValue
    case modelResult of
        Ok model -> putStrLn . render $ withContext template model
        Error err -> hPutStrLn stderr $ "Error while parsing JSON: " ++ err

getTemplate :: Config -> IO (StringTemplate String)
getTemplate config = case groupPath config of
    Nothing -> newSTMP <$> readFile name
    Just templateGroup -> do
        group <- directoryGroup templateGroup
        case getStringTemplate name group of
            Just template -> return template
            Nothing -> hPutStrLn stderr ("Error trying to get template " ++ name) >> exitFailure
    where name = templateName config

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
