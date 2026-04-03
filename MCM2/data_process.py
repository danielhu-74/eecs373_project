import pandas as pd
import numpy as np
import chardet
import re
def detect_encoding(file_path):
    with open(file_path, 'rb') as f:
        raw_data = f.read()
        result = chardet.detect(raw_data)
        return result['encoding']

# use Pandas to read CSV file directly
def read_csv(file_path):
    encoding = detect_encoding(file_path)
    return pd.read_csv(file_path, encoding=encoding)
# load data
data_dict = read_csv('./2025_Problem_C_Data/data_dictionary.csv')
athletes = read_csv('./2025_Problem_C_Data/summerOly_athletes.csv')
hosts = read_csv('./2025_Problem_C_Data/summerOly_hosts.csv')
medals = read_csv('./2025_Problem_C_Data/summerOly_medal_counts.csv')
programs = read_csv('./2025_Problem_C_Data/summerOly_programs.csv')

# Fill in missing values (for example, fill in missing values of numerical data with the 0)
athletes.fillna(0, inplace=True)

# Remove special characters from column names
medals['Gold'] = medals['Gold'].astype(int) # 确保奖牌数为整数
medals = medals.drop('Silver', axis=1)
medals = medals.drop('Bronze', axis=1)

# delete duplicates
athletes.drop_duplicates(inplace=True)

# Merge data
merged_data = pd.merge(medals, hosts, on="Year", how="left")
print("done1")

# Add new columns
merged_data['Country'] = merged_data['Host'].str.split(',').str[-1].str.strip()
merged_data['is_host'] = (merged_data['NOC'] == merged_data['Country']).astype(int)

# Calculate cumulative medals
total_medals_by_year_country = medals.groupby(['Year', 'NOC']).agg(Total_Medals=('Total', 'sum'), Gold_Medals=('Gold', 'sum')).reset_index()
total_medals_by_year_country['Cumulative_Total_Medals'] = total_medals_by_year_country.groupby('NOC')['Total_Medals'].cumsum()
total_medals_by_year_country['Cumulative_Gold_Medals'] = total_medals_by_year_country.groupby('NOC')['Gold_Medals'].cumsum()

# Merge cumulative medals data
merged_data = pd.merge(merged_data, total_medals_by_year_country[['Year', 'NOC', 'Cumulative_Total_Medals', 'Cumulative_Gold_Medals']], on=['Year', 'NOC'], how='left')

total_programs = pd.DataFrame({
    'Year': programs.columns[4:],  # from the 5th column extract the year
    'Total Events': programs.iloc[71, 4:],  # from the 71st row extract the total number of events
    'Total Disciplines': programs.iloc[72, 4:],  # from the 72nd row extract the total number of disciplines
    'Total Sports': programs.iloc[73, 4:]  # from the 73rd row extract the total number of sports
})
total_programs['Year'] = total_programs['Year'].str.replace(r'\D', '', regex=True)

# Convert data types
total_programs['Year'] = total_programs['Year'].astype(int)
total_programs['Total Events'] = total_programs['Total Events'].astype(float)
total_programs['Total Disciplines'] = total_programs['Total Disciplines'].astype(float)
total_programs['Total Sports'] = total_programs['Total Sports'].astype(float)

merged_data['Year'] = merged_data['Year'].astype(int)

# Merge total programs data
merged_data = pd.merge(merged_data, total_programs, on='Year', how='left')

# map country names
country_mapping = {
    'Soviet Union': 'Russia',
    'West Germany': 'Germany',
    'East Germany': 'Germany',
    'Yugoslavia': 'Serbia',
    'Czechoslovakia': 'Czech Republic',
    'Bohemia': 'Czech Republic',
    'Russian Empire': 'Russia',
    'United Team of Germany': 'Germany',
    'Unified Team': 'Russia',
    'Serbia and Montenegro': 'Serbia',
    'Netherlands Antilles': 'Netherlands',
    'Virgin Islands': 'United States',
}

noc_mapping = {
    'URS': 'RUS',
    'EUA': 'GER',
    'FRG': 'GER',
    'GDR': 'GER',
    'YUG': 'SRB',
    'TCH': 'CZE',
    'BOH': 'CZE',
    'EUN': 'RUS',
    'SCG': 'SRB',
    'ANZ': 'AUS',
    'NBO': 'KEN',
    'WIF': 'USA',
    'IOP': 'IOA',
}

# map country names
country_codes = {
    'AFG': 'Afghanistan',
    'ALB': 'Albania',
    'ALG': 'Algeria',
    'AND': 'Andorra',
    'ANG': 'Angola',
    'ANT': 'Antigua and Barbuda',
    'ARG': 'Argentina',
    'ARM': 'Armenia',
    'ARU': 'Aruba',
    'ASA': 'American Samoa',
    'AUS': 'Australia',
    'AUT': 'Austria',
    'AZE': 'Azerbaijan',
    'BAH': 'Bahamas',
    'BAN': 'Bangladesh',
    'BAR': 'Barbados',
    'BDI': 'Burundi',
    'BEL': 'Belgium',
    'BEN': 'Benin',
    'BER': 'Bermuda',
    'BHU': 'Bhutan',
    'BIH': 'Bosnia and Herzegovina',
    'BIZ': 'Belize',
    'BLR': 'Belarus',
    'BOL': 'Bolivia',
    'BOT': 'Botswana',
    'BRA': 'Brazil',
    'BRN': 'Bahrain',
    'BRU': 'Brunei',
    'BUL': 'Bulgaria',
    'BUR': 'Burkina Faso',
    'CAF': 'Central African Republic',
    'CAM': 'Cambodia',
    'CAN': 'Canada',
    'CAY': 'Cayman Islands',
    'CGO': 'Congo',
    'CHA': 'Chad',
    'CHI': 'Chile',
    'CHN': 'China',
    'CIV': 'Ivory Coast',
    'CMR': 'Cameroon',
    'COD': 'Democratic Republic of the Congo',
    'COK': 'Cook Islands',
    'COL': 'Colombia',
    'COM': 'Comoros',
    'CPV': 'Cape Verde',
    'CRC': 'Costa Rica',
    'CRO': 'Croatia',
    'CUB': 'Cuba',
    'CYP': 'Cyprus',
    'CZE': 'Czech Republic',
    'DEN': 'Denmark',
    'DJI': 'Djibouti',
    'DMA': 'Dominica',
    'DOM': 'Dominican Republic',
    'ECU': 'Ecuador',
    'EGY': 'Egypt',
    'ERI': 'Eritrea',
    'ESA': 'El Salvador',
    'ESP': 'Spain',
    'EST': 'Estonia',
    'ETH': 'Ethiopia',
    'FIJ': 'Fiji',
    'FIN': 'Finland',
    'FRA': 'France',
    'FSM': 'Micronesia',
    'GAB': 'Gabon',
    'GAM': 'Gambia',
    'GBR': 'Great Britain',
    'GBS': 'Guinea-Bissau',
    'GEO': 'Georgia',
    'GEQ': 'Equatorial Guinea',
    'GER': 'Germany',
    'GHA': 'Ghana',
    'GRE': 'Greece',
    'GRN': 'Grenada',
    'GUA': 'Guatemala',
    'GUI': 'Guinea',
    'GUM': 'Guam',
    'GUY': 'Guyana',
    'HAI': 'Haiti',
    'HKG': 'Hong Kong',
    'HON': 'Honduras',
    'HUN': 'Hungary',
    'INA': 'Indonesia',
    'IND': 'India',
    'IRI': 'Iran',
    'IRL': 'Ireland',
    'IRQ': 'Iraq',
    'ISL': 'Iceland',
    'ISR': 'Israel',
    'ISV': 'Virgin Islands',
    'ITA': 'Italy',
    'IVB': 'British Virgin Islands',
    'JAM': 'Jamaica',
    'JOR': 'Jordan',
    'JPN': 'Japan',
    'KAZ': 'Kazakhstan',
    'KEN': 'Kenya',
    'KGZ': 'Kyrgyzstan',
    'KIR': 'Kiribati',
    'KOR': 'South Korea',
    'KOS': 'Kosovo',
    'KSA': 'Saudi Arabia',
    'KUW': 'Kuwait',
    'LAO': 'Laos',
    'LAT': 'Latvia',
    'LBA': 'Libya',
    'LBR': 'Liberia',
    'LCA': 'Saint Lucia',
    'LES': 'Lesotho',
    'LIE': 'Liechtenstein',
    'LTU': 'Lithuania',
    'LUX': 'Luxembourg',
    'MAD': 'Madagascar',
    'MAR': 'Morocco',
    'MAS': 'Malaysia',
    'MAW': 'Malawi',
    'MDA': 'Moldova',
    'MDV': 'Maldives',
    'MEX': 'Mexico',
    'MGL': 'Mongolia',
    'MHL': 'Marshall Islands',
    'MKD': 'North Macedonia',
    'MLI': 'Mali',
    'MLT': 'Malta',
    'MNE': 'Montenegro',
    'MON': 'Monaco',
    'MOZ': 'Mozambique',
    'MRI': 'Mauritius',
    'MTN': 'Mauritania',
    'MYA': 'Myanmar',
    'NAM': 'Namibia',
    'NCA': 'Nicaragua',
    'NED': 'Netherlands',
    'NEP': 'Nepal',
    'NGR': 'Nigeria',
    'NIG': 'Niger',
    'NOR': 'Norway',
    'NRU': 'Nauru',
    'NZL': 'New Zealand',
    'OMA': 'Oman',
    'PAK': 'Pakistan',
    'PAN': 'Panama',
    'PAR': 'Paraguay',
    'PER': 'Peru',
    'PHI': 'Philippines',
    'PLE': 'Palestine',
    'PLW': 'Palau',
    'PNG': 'Papua New Guinea',
    'POL': 'Poland',
    'POR': 'Portugal',
    'PRK': 'North Korea',
    'PUR': 'Puerto Rico',
    'QAT': 'Qatar',
    'ROU': 'Romania',
    'RSA': 'South Africa',
    'RUS': 'Russia',
    'RWA': 'Rwanda',
    'SAM': 'Samoa',
    'SEN': 'Senegal',
    'SEY': 'Seychelles',
    'SIN': 'Singapore',
    'SKN': 'Saint Kitts and Nevis',
    'SLE': 'Sierra Leone',
    'SLO': 'Slovenia',
    'SMR': 'San Marino',
    'SOL': 'Solomon Islands',
    'SRB': 'Serbia',
    'SRI': 'Sri Lanka',
    'STP': 'Sao Tome and Principe',
    'SUD': 'Sudan',
    'SUI': 'Switzerland',
    'SUR': 'Suriname',
    'SVK': 'Slovakia',
    'SWE': 'Sweden',
    'SWZ': 'Eswatini',
    'SYR': 'Syria',
    'TAN': 'Tanzania',
    'TGA': 'Tonga',
    'THA': 'Thailand',
    'TJK': 'Tajikistan',
    'TKM': 'Turkmenistan',
    'TLS': 'Timor-Leste',
    'TOG': 'Togo',
    'TPE': 'Chinese Taipei',
    'TTO': 'Trinidad and Tobago',
    'TUN': 'Tunisia',
    'TUR': 'Turkey',
    'TUV': 'Tuvalu',
    'UAE': 'United Arab Emirates',
    'UGA': 'Uganda',
    'UKR': 'Ukraine',
    'URU': 'Uruguay',
    'USA': 'United States',
    'UZB': 'Uzbekistan',
    'VAN': 'Vanuatu',
    'VEN': 'Venezuela',
    'VIE': 'Vietnam',
    'VIN': 'Saint Vincent and the Grenadines',
    'YEM': 'Yemen',
    'ZAM': 'Zambia',
    'ZIM': 'Zimbabwe'
}

athletes['NOC'] = athletes['NOC'].replace(noc_mapping)
athletes['NOC'] = athletes['NOC'].map(country_codes).fillna(athletes['NOC'])
athlete_counts = athletes.groupby(['Year', 'NOC']).size().reset_index(name='Athlete Count')
# calculate total participations
participation_counts = athletes.groupby(['Year', 'NOC']).size().reset_index(name='Athlete Count')
participation_counts['Athlete Count'] = participation_counts['Athlete Count'].fillna(0)
total_participations = participation_counts.groupby('NOC')['Year'].count().reset_index(name='Total Participations')
total_participations['Total Participations'] = total_participations['Total Participations'].fillna(0)

# Merge athlete counts and total participations data
merged_data = pd.merge(merged_data, athlete_counts, on=["Year", "NOC"], how="left")
merged_data = pd.merge(merged_data, total_participations, on="NOC", how="left")
merged_data['Athlete Count'] = merged_data['Athlete Count'].fillna(0)
merged_data['Total Participations'] = merged_data['Total Participations'].fillna(0)



